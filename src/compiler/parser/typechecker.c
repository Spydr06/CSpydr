#include "typechecker.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "ast/ast.h"
#include "error/error.h"
#include "globals.h"
#include "optimizer/constexpr.h"
#include "parser/validator.h"
#include "utils.h"
#include "codegen/codegen_utils.h"
#include "ast/ast_iterator.h"

#define GET_VALIDATOR(va) Validator_T* v = va_arg(va, Validator_T*)

static void set_fn(ASTObj_T* fn, va_list args);
static void unset_fn(ASTObj_T* fn, va_list args);

static ASTNode_T* typecheck_arg_pass(ASTType_T* expected, ASTNode_T* received);
static void typecheck_call(ASTNode_T* call, va_list args);
static void typecheck_explicit_cast(ASTNode_T* cast, va_list args);
static void typecheck_assignment(ASTNode_T* assignment, va_list args);
static void typecheck_array_lit(ASTNode_T* a_lit, va_list args);
static void typecheck_struct_lit(ASTNode_T* a_lit, va_list args);
static void typecheck_inc(ASTNode_T* inc, va_list args);
static void typecheck_dec(ASTNode_T* dec, va_list args);

static const ASTIteratorList_T iterator = {
    .obj_start_fns = {
        [OBJ_FUNCTION] = set_fn,
    },
    .obj_end_fns = {
        [OBJ_FUNCTION] = unset_fn
    },
    .node_end_fns = {
        [ND_CALL] = typecheck_call,
        [ND_CAST] = typecheck_explicit_cast,
        [ND_ASSIGN] = typecheck_assignment,
        [ND_ARRAY] = typecheck_array_lit,
        [ND_STRUCT] = typecheck_struct_lit,
        [ND_INC] = typecheck_inc,
        [ND_DEC] = typecheck_dec
    }
};

void run_typechecker(ASTProg_T* ast, Validator_T* v)
{
    ast_iterate(&iterator, ast, v);
}

static void set_fn(ASTObj_T* fn, va_list args)
{
    GET_VALIDATOR(args);
    v->current_function = fn;
}

static void unset_fn(ASTObj_T* fn, va_list args)
{
    GET_VALIDATOR(args);
    v->current_function = NULL;
}

static void typecheck_call(ASTNode_T* call, va_list args)
{
    ASTType_T* call_type = unpack(call->expr->data_type);
    size_t expected_arg_num = call_type->arg_types->size;
    for(size_t i = 0; i < MIN(expected_arg_num, call->args->size); i++)
        call->args->items[i] = typecheck_arg_pass(call_type->arg_types->items[i], call->args->items[i]);
}

static void typecheck_assignment(ASTNode_T* assignment, va_list args)
{
    char buf1[BUFSIZ] = {};
    char buf2[BUFSIZ] = {};

    if(unpack(assignment->left->data_type)->is_constant && !assignment->is_initializing)
        throw_error(
            ERR_TYPE_ERROR_UNCR, 
            assignment->tok, 
            "cannot assign new values to constant type `%s`", 
            ast_type_to_str(buf1, assignment->left->data_type, LEN(buf1))
        );

    if(types_equal(assignment->left->data_type, assignment->right->data_type))
        return;

    if(implicitly_castable(assignment->tok, assignment->right->data_type, assignment->left->data_type) != CAST_ERR)
    {
        assignment->right = implicit_cast(assignment->tok, assignment->right, assignment->left->data_type);
        return;
    }

    if(unpack(assignment->right->data_type)->kind == TY_ARRAY &&
        unpack(assignment->left->data_type)->kind == TY_VLA)
    {
        // todo    
    }

    throw_error(ERR_TYPE_ERROR_UNCR, assignment->tok, "assignment type missmatch: cannot assign `%s` to `%s`", 
        ast_type_to_str(buf1, assignment->right->data_type, LEN(buf1)),
        ast_type_to_str(buf2, assignment->left->data_type, LEN(buf2))    
    );
}

static ASTNode_T* typecheck_arg_pass(ASTType_T* expected, ASTNode_T* received)
{
    if(types_equal(expected, received->data_type))
        return received;
    
    if(implicitly_castable(received->tok, received->data_type, expected) == CAST_OK)
        return implicit_cast(received->tok, received, expected);
    
    char buf1[BUFSIZ] = {};
    char buf2[BUFSIZ] = {};
    throw_error(ERR_TYPE_ERROR_UNCR, received->tok, "cannot implicitly cast from `%s` to `%s`", 
        ast_type_to_str(buf1, received->data_type, LEN(buf1)),
        ast_type_to_str(buf2, expected, LEN(buf2))    
    );

    return received;
}

static void typecheck_explicit_cast(ASTNode_T* cast, va_list args)
{
    // Buffer for warnings and errors
    char buf1[BUFSIZ] = {};

    if(types_equal(cast->left->data_type, cast->data_type) && !cast->data_type->no_warnings)
    {
        throw_error(ERR_TYPE_CAST_WARN, cast->tok, "unnecessary type cast: expression is already of type `%s`",
            ast_type_to_str(buf1, cast->data_type, LEN(buf1))
        );
        return;
    }

    ASTType_T* from = unpack(cast->left->data_type);
    ASTType_T* to = unpack(cast->data_type);

    if(from->kind == TY_VOID)
        throw_error(ERR_TYPE_ERROR_UNCR, cast->tok, "cannot cast from `void` to `%s`", 
            ast_type_to_str(buf1, cast->data_type, LEN(buf1))
        );
    
    if(to->kind == TY_VOID)
        throw_error(ERR_TYPE_ERROR_UNCR, cast->tok, "cannot cast from `%s` to `void`", 
            ast_type_to_str(buf1, cast->left->data_type, LEN(buf1))
        );
}

static void typecheck_array_lit(ASTNode_T* a_lit, va_list args)
{
    ASTType_T* base_ty = unpack(a_lit->data_type->base);
    
    char buf1[BUFSIZ] = {'\0'};
    char buf2[BUFSIZ] = {'\0'};

    for(size_t i = 0; i < a_lit->args->size; i++)
    {
        ASTNode_T* arg = a_lit->args->items[i];

        if(types_equal(arg->data_type, base_ty))
            continue;

        if(implicitly_castable(arg->tok, arg->data_type, base_ty) == CAST_OK)
            a_lit->args->items[i] = implicit_cast(arg->tok, arg, base_ty);
        else
            throw_error(ERR_TYPE_ERROR_UNCR, arg->tok, "cannot implicitly cast from `%s` to `%s`",
                ast_type_to_str(buf1, arg->data_type, BUFSIZ),
                ast_type_to_str(buf2, base_ty, BUFSIZ)
            );
    }
}

static void typecheck_struct_lit(ASTNode_T* s_lit, va_list args)
{
    size_t expected_len = unpack(s_lit->data_type)->members->size;
    size_t got_len = s_lit->args->size;

    char buf1[BUFSIZ] = {'\0'};
    char buf2[BUFSIZ] = {'\0'};

    if(got_len > expected_len) 
    {
        throw_error(ERR_TYPE_ERROR_UNCR, s_lit->tok, "too many struct members for specified type `%s`", ast_type_to_str(buf1, s_lit->data_type, LEN(buf1)));
        memset(buf1, '\0', BUFSIZ * sizeof(char));
    }

    for(size_t i = 0; i < MIN(expected_len, got_len); i++)
    {
        ASTNode_T* arg = s_lit->args->items[i];
        ASTType_T* expected_ty = ((ASTNode_T*) unpack(s_lit->data_type)->members->items[i])->data_type;

        if(types_equal(arg->data_type, expected_ty))
            continue;

        if(implicitly_castable(arg->tok, arg->data_type, expected_ty) == CAST_OK)
            s_lit->args->items[i] = implicit_cast(arg->tok, arg, expected_ty);
        else
            throw_error(ERR_TYPE_ERROR_UNCR, arg->tok, "cannot implicitly cast from `%s` to `%s`",
                ast_type_to_str(buf1, arg->data_type, BUFSIZ),
                buf2[0] == '\0' ? buf2 : ast_type_to_str(buf2, expected_ty, BUFSIZ)
            );
    }
}

static void typecheck_inc(ASTNode_T* inc, va_list args)
{
    char buf[BUFSIZ] = {'\0'};
    if(inc->data_type->is_constant)
        throw_error(ERR_TYPE_ERROR_UNCR, inc->tok, "cannot increment constant type `%s`", ast_type_to_str(buf, inc->data_type, LEN(buf)));
}

static void typecheck_dec(ASTNode_T* dec, va_list args)
{
    char buf[BUFSIZ] = {'\0'};
    if(dec->data_type->is_constant)
        throw_error(ERR_TYPE_ERROR_UNCR, dec->tok, "cannot decrement constant type `%s`", ast_type_to_str(buf, dec->data_type, LEN(buf)));
}

bool types_equal(ASTType_T* a, ASTType_T* b)
{
    if(!a || !b || a->kind != b->kind || a->is_constant != b->is_constant)
        return false;
    
    switch(a->kind)
    {
        case TY_C_ARRAY:
        case TY_PTR:
            return types_equal(a->base, b->base);
        
        case TY_STRUCT:
            if(a->members->size != b->members->size || a->is_union != b->is_union)
                return false;
            for(size_t i = 0; i < a->members->size; i++)
            {
                ASTNode_T* am = a->members->items[i];
                ASTNode_T* bm = b->members->items[i];
                if(!types_equal(am->data_type, bm->data_type))
                    return false;
            }
            return true;
        
        case TY_ENUM:
            if(a->members->size != b->members->size)
                return false;
            for(size_t i = 0; i < a->members->size; i++)
            {
                ASTObj_T* am = a->members->items[i];
                ASTObj_T* bm = b->members->items[i];
                if(!identifiers_equal(am->id, bm->id) || const_i64(am->value) != const_i64(bm->value))
                    return false;
            }
            return true;

        case TY_UNDEF:
            return identifiers_equal(a->id, b->id);
        
        case TY_FN:
            if(a->arg_types->size != b->arg_types->size || !types_equal(a->base, b->base))
                return false;
            for(size_t i = 0; i < a->arg_types->size; i++)
                if(!types_equal(a->arg_types->items[i], b->arg_types->items[i]))
                    return false;
            return true;

        default:
            return true;
    }
}

enum IMPLICIT_CAST_RESULT implicitly_castable(Token_T* tok, ASTType_T* from, ASTType_T* to)
{
    from = unpack(from);
    to = unpack(to);

    if(!from || !to)
        return CAST_ERR;

    if(from->base && to->base && from->base->is_constant && !to->base->is_constant)
        return CAST_DELETING_CONST;
    
    // Buffer for warnings and errors
    char buf1[BUFSIZ] = {};
    char buf2[BUFSIZ] = {};

    if(is_integer(from) && is_integer(to))
    {
        //if(from->size > to->size)
        //    throw_error(ERR_TYPE_CAST_WARN, tok, "implicitly casting from `%s` to `%s`: possible data loss",
        //        ast_type_to_str(buf1, from, LEN(buf1)),
        //        ast_type_to_str(buf2, to, LEN(buf2))
        //    );
        return CAST_OK;
    }
    if(is_flonum(from) && is_flonum(to))
        return CAST_OK;
    if(is_integer(from) && is_flonum(to))
        return CAST_OK;
    if(is_flonum(from) && is_integer(to))
    {
        throw_error(ERR_TYPE_CAST_WARN, tok, "implicitly casting from `%s` to `%s`",
            ast_type_to_str(buf1, from, LEN(buf1)),
            ast_type_to_str(buf2, to, LEN(buf2))
        );
        return CAST_OK;
    }
    if((from->kind == TY_PTR || from->kind == TY_C_ARRAY) && to->kind == TY_PTR)
        return CAST_OK;
    if(from->kind == TY_ARRAY && to->kind == TY_VLA)
        return CAST_OK;
    if(from->kind == TY_PTR && unpack(from->base)->kind == TY_ARRAY && to->kind == TY_VLA)
        return types_equal(unpack(from->base)->base, to->base) ? CAST_OK : CAST_ERR;
    if(from->kind == TY_STRUCT && to->kind == TY_STRUCT)
        return types_equal(from, to) ? CAST_OK : CAST_ERR;
    return CAST_ERR;
}

ASTNode_T* implicit_cast(Token_T* tok, ASTNode_T* expr, ASTType_T* to)
{
    if(unpack(expr->data_type)->kind == TY_ARRAY && to->kind == TY_VLA)
    {
        ASTNode_T* ref = init_ast_node(ND_REF, tok);
        ref->data_type = to;
        ref->right = expr;
        return ref;
    }

    ASTNode_T* cast = init_ast_node(ND_CAST, tok);
    cast->data_type = to;
    cast->left = expr;
    return cast;
}