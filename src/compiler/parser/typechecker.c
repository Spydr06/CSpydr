#include "typechecker.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "ast/ast.h"
#include "config.h"
#include "error/error.h"
#include "lexer/token.h"
#include "list.h"
#include "optimizer/constexpr.h"
#include "utils.h"
#include "codegen/codegen_utils.h"
#include "ast/ast_iterator.h"
#include "ast/types.h"
#include "context.h"

#define GET_TYPECHECKER(va) TypeChecker_T* t = va_arg(va, TypeChecker_T*)

static void set_fn(ASTObj_T* fn, va_list args);
static void unset_fn(ASTObj_T* fn, va_list args);

static ASTNode_T* typecheck_arg_pass(TypeChecker_T* t, ASTType_T* expected, ASTNode_T* received);
static void typecheck_call(ASTNode_T* call, va_list args);
static void typecheck_explicit_cast(ASTNode_T* cast, va_list args);
static void typecheck_assignment(ASTNode_T* assignment, va_list args);
static void typecheck_array_lit(ASTNode_T* a_lit, va_list args);
static void typecheck_struct_lit(ASTNode_T* a_lit, va_list args);
static void typecheck_inc(ASTNode_T* inc, va_list args);
static void typecheck_dec(ASTNode_T* dec, va_list args);
static void typecheck_for_range(ASTNode_T* loop, va_list args);
static void typecheck_lambda_entry(ASTNode_T* lambda, va_list args);
static void typecheck_lambda_exit(ASTNode_T* lambda, va_list args);
static void typecheck_return(ASTNode_T* ret, va_list args);
static void typecheck_binop(ASTNode_T* op, va_list args);
bool types_equal_strict(Context_T* context, ASTType_T* a, ASTType_T* b);

static const ASTIteratorList_T iterator = {
    .obj_start_fns = {
        [OBJ_FUNCTION] = set_fn
    },
    .obj_end_fns = {
        [OBJ_FUNCTION] = unset_fn
    },
    .node_start_fns = {
        [ND_LAMBDA] = typecheck_lambda_entry,
    },
    .node_end_fns = {
        [ND_CALL] = typecheck_call,
        [ND_CAST] = typecheck_explicit_cast,
        [ND_ASSIGN] = typecheck_assignment,
        [ND_ARRAY] = typecheck_array_lit,
        [ND_STRUCT] = typecheck_struct_lit,
        [ND_INC] = typecheck_inc,
        [ND_DEC] = typecheck_dec,
        [ND_FOR_RANGE] = typecheck_for_range,
        [ND_RETURN] = typecheck_return,
        [ND_LAMBDA] = typecheck_lambda_exit,
        [ND_ADD] = typecheck_binop,
        [ND_SUB] = typecheck_binop,
        [ND_MUL] = typecheck_binop,
        [ND_DIV] = typecheck_binop
    }
};

void typechecker_init(TypeChecker_T* t, Context_T* context)
{
    t->context = context;
    t->current_fn = NULL;
    t->return_type_stack = init_list();
}

void typechecker_free(TypeChecker_T* t)
{
    free_list(t->return_type_stack);
}

void typecheck_obj(TypeChecker_T* t, ASTObj_T* obj)
{
    ASTObj_T** old = t->context->current_obj;
    t->context->current_obj = &t->current_fn;

    ast_iterate_obj(&iterator, obj, t);
    
    t->context->current_obj = old;
}

static inline ASTType_T* current_return_type(TypeChecker_T* t)
{
    return list_last(t->return_type_stack);
}

static inline void push_return_type(TypeChecker_T* t, ASTType_T* type)
{
    list_push(t->return_type_stack, type);
}

static inline ASTType_T* pop_return_type(TypeChecker_T* t)
{
    return list_pop(t->return_type_stack);
}

static void set_fn(ASTObj_T* fn, va_list args)
{
    GET_TYPECHECKER(args);
    t->current_fn = fn;
    push_return_type(t, fn->return_type);
}

static void unset_fn(ASTObj_T* fn, va_list args)
{
    GET_TYPECHECKER(args);
    t->current_fn = NULL;
    ASTType_T* popped = pop_return_type(t);
    assert(fn->return_type == popped);
}

static bool is_const_len_array(ASTType_T* arr)
{
    arr = unpack(arr);
    return arr->kind == TY_ARRAY || arr->kind == TY_C_ARRAY;
}

static void typecheck_call(ASTNode_T* call, va_list args)
{
    GET_TYPECHECKER(args);
    ASTType_T* call_type = unpack(call->expr->data_type);
    size_t expected_arg_num = call_type->arg_types->size;

    for(size_t i = 0; i < MIN(expected_arg_num, call->args->size); i++)
        call->args->items[i] = typecheck_arg_pass(t, call_type->arg_types->items[i], call->args->items[i]);

    if(expected_arg_num < call->args->size) {
        for(size_t i = expected_arg_num; i < call->args->size; i++) {
            ASTNode_T* arg = call->args->items[i];
            if(arg->unpack_mode)
            {
                char buf[BUFSIZ];
                if(!is_const_len_array(arg->data_type) && unpack(arg->data_type)->kind != TY_STRUCT)
                    throw_error(t->context, ERR_TYPE_ERROR_UNCR, arg->tok, "unpacking with `...` is not supported for type `%s`", ast_type_to_str(t->context, buf, arg->data_type, LEN(buf)));
            }
        }
    }
}

static void typecheck_for_range(ASTNode_T* loop, va_list args)
{
    GET_TYPECHECKER(args);
    loop->left = implicit_cast(t->context, loop->left->tok, loop->left, (ASTType_T*) primitives[TY_I64]);
    loop->right = implicit_cast(t->context, loop->right->tok, loop->right, (ASTType_T*) primitives[TY_I64]);
}

static void typecheck_assignment(ASTNode_T* assignment, va_list args)
{
    GET_TYPECHECKER(args);
    char buf1[BUFSIZ] = {'\0'};
    char buf2[BUFSIZ] = {'\0'};

    if(unpack(assignment->left->data_type)->is_constant && !assignment->is_initializing)
        throw_error(t->context, 
            ERR_TYPE_ERROR_UNCR, 
            assignment->tok, 
            "cannot assign new values to constant type `%s`", 
            ast_type_to_str(t->context, buf1, assignment->left->data_type, LEN(buf1))
        );

    if(types_equal(t->context, assignment->left->data_type, assignment->right->data_type))
        return;

    if(implicitly_castable(t->context, assignment->tok, assignment->right->data_type, assignment->left->data_type) != CAST_ERR)
    {
        assignment->right = implicit_cast(t->context, assignment->tok, assignment->right, assignment->left->data_type);
        return;
    }

    if(unpack(assignment->right->data_type)->kind == TY_ARRAY &&
        unpack(assignment->left->data_type)->kind == TY_VLA)
    {
        // todo    
    }

    throw_error(t->context, ERR_TYPE_ERROR_UNCR, assignment->tok, "assignment type missmatch: cannot assign `%s` to `%s`", 
        ast_type_to_str(t->context, buf1, assignment->right->data_type, LEN(buf1)),
        ast_type_to_str(t->context, buf2, assignment->left->data_type, LEN(buf2))    
    );
}

static ASTNode_T* typecheck_arg_pass(TypeChecker_T* t, ASTType_T* expected, ASTNode_T* received)
{
    char buf1[BUFSIZ] = {'\0'};
    if(received->unpack_mode)
    {
        if(!is_const_len_array(received->data_type))
            throw_error(t->context, ERR_TYPE_ERROR_UNCR, received->tok, "unpacking with `...` is not supported for type `%s`\n", ast_type_to_str(t->context, buf1, received->data_type, LEN(buf1)));
        throw_error(t->context, ERR_TYPE_ERROR_UNCR, received->tok, "unpacking with `...` is only available for variable-length argument lists");
        return received;
    }

    if(types_equal(t->context, expected, received->data_type))
        return received;
    
    if(implicitly_castable(t->context, received->tok, received->data_type, expected) == CAST_OK)
        return implicit_cast(t->context, received->tok, received, expected);
    
    char buf2[BUFSIZ] = {'\0'};
    throw_error(t->context, ERR_TYPE_ERROR_UNCR, received->tok, "cannot implicitly cast from `%s` to `%s`", 
        ast_type_to_str(t->context, buf1, received->data_type, LEN(buf1)),
        ast_type_to_str(t->context, buf2, expected, LEN(buf2))    
    );

    return received;
}

static void typecheck_explicit_cast(ASTNode_T* cast, va_list args)
{
    GET_TYPECHECKER(args);
    // Buffer for warnings and errors
    char buf1[BUFSIZ] = {'\0'};

    if(types_equal_strict(t->context, cast->left->data_type, cast->data_type) && !cast->data_type->no_warnings)
    {
        //if((cast->left->tok && cast->left->tok->in_macro_expansion) ||
        //    (cast->right->tok && cast->right->tok->in_macro_expansion))
        //    return;   
        if(cast->left->data_type == cast->data_type && cast->data_type->kind == TY_STRUCT)
            return; // skip pointer-identical anonymous structs

        throw_error(t->context, ERR_TYPE_CAST_WARN, cast->tok, "unnecessary type cast: expression is already of type `%s`",
            ast_type_to_str(t->context, buf1, cast->data_type, LEN(buf1))
        );
        return;
    }

    ASTType_T* from = unpack(cast->left->data_type);
    ASTType_T* to = unpack(cast->data_type);

    if(from->kind == TY_VOID)
        throw_error(t->context, ERR_TYPE_ERROR_UNCR, cast->tok, "cannot cast from `void` to `%s`", 
            ast_type_to_str(t->context, buf1, cast->data_type, LEN(buf1))
        );
    
    if(to->kind == TY_VOID)
        throw_error(t->context, ERR_TYPE_ERROR_UNCR, cast->tok, "cannot cast from `%s` to `void`", 
            ast_type_to_str(t->context, buf1, cast->left->data_type, LEN(buf1))
        );
}

static void typecheck_array_lit(ASTNode_T* a_lit, va_list args)
{
    GET_TYPECHECKER(args);
    ASTType_T* base_ty = unpack(a_lit->data_type->base);
    
    char buf1[BUFSIZ] = {'\0'};
    char buf2[BUFSIZ] = {'\0'};

    for(size_t i = 0; i < a_lit->args->size; i++)
    {
        ASTNode_T* arg = a_lit->args->items[i];
        ASTType_T* arg_type = arg->data_type;

        if(arg->unpack_mode)
        {
            if(!is_const_len_array(arg_type)) {
                throw_error(t->context, ERR_TYPE_ERROR_UNCR, arg->tok, "unpacking with `...` is not supported for type `%s`", ast_type_to_str(t->context, buf1, arg_type, LEN(buf1)));
                continue;
            }

            arg_type = unpack(arg_type)->base;
        }

        if(types_equal(t->context, arg_type, base_ty))
            continue;

        if(implicitly_castable(t->context, arg->tok, arg_type, base_ty) == CAST_OK)
            a_lit->args->items[i] = implicit_cast(t->context, arg->tok, arg, base_ty);
        else {
            memset(buf1, 0, LEN(buf1));
            memset(buf2, 0, LEN(buf2));
            throw_error(t->context, ERR_TYPE_ERROR_UNCR, arg->tok, "cannot implicitly cast from `%s` to `%s`",
                ast_type_to_str(t->context, buf1, arg_type, BUFSIZ),
                ast_type_to_str(t->context, buf2, base_ty, BUFSIZ)
            );
        }
    }
}

static void typecheck_struct_lit(ASTNode_T* s_lit, va_list args)
{
    GET_TYPECHECKER(args);
    size_t expected_len = unpack(s_lit->data_type)->members->size;
    size_t got_len = s_lit->args->size;

    char buf1[BUFSIZ] = {'\0'};
    char buf2[BUFSIZ] = {'\0'};

    if(got_len > expected_len) 
    {
        throw_error(t->context, ERR_TYPE_ERROR_UNCR, s_lit->tok, "too many struct members for specified type `%s`", ast_type_to_str(t->context, buf1, s_lit->data_type, LEN(buf1)));
        memset(buf1, '\0', BUFSIZ * sizeof(char));
    }

    for(size_t i = 0; i < MIN(expected_len, got_len); i++)
    {
        ASTNode_T* arg = s_lit->args->items[i];
        ASTType_T* expected_ty = ((ASTNode_T*) unpack(s_lit->data_type)->members->items[i])->data_type;

        if(types_equal(t->context, arg->data_type, expected_ty))
            continue;

        if(implicitly_castable(t->context, arg->tok, arg->data_type, expected_ty) == CAST_OK)
            s_lit->args->items[i] = implicit_cast(t->context, arg->tok, arg, expected_ty);
        else {
            memset(buf1, 0, LEN(buf1));
            memset(buf2, 0, LEN(buf2));

            throw_error(t->context, ERR_TYPE_ERROR_UNCR, arg->tok, "cannot implicitly cast from `%s` to `%s`",
                ast_type_to_str(t->context, buf1, arg->data_type, BUFSIZ),
                buf2[0] == '\0' ? buf2 : ast_type_to_str(t->context, buf2, expected_ty, BUFSIZ)
            );
        }
    }
}

static void typecheck_inc(ASTNode_T* inc, va_list args)
{
    GET_TYPECHECKER(args);
    char buf[BUFSIZ] = {'\0'};
    if(inc->data_type->is_constant)
        throw_error(t->context, ERR_TYPE_ERROR_UNCR, inc->tok, "cannot increment constant type `%s`", ast_type_to_str(t->context, buf, inc->data_type, LEN(buf)));
}

static void typecheck_dec(ASTNode_T* dec, va_list args)
{
    GET_TYPECHECKER(args);
    char buf[BUFSIZ] = {'\0'};
    if(dec->data_type->is_constant)
        throw_error(t->context, ERR_TYPE_ERROR_UNCR, dec->tok, "cannot decrement constant type `%s`", ast_type_to_str(t->context, buf, dec->data_type, LEN(buf)));
}

static void typecheck_lambda_entry(ASTNode_T* lambda, va_list args)
{
    GET_TYPECHECKER(args);
    push_return_type(t, lambda->data_type->base);
}

static void typecheck_lambda_exit(ASTNode_T* lambda, va_list args)
{
    GET_TYPECHECKER(args);
    ASTType_T* popped = pop_return_type(t);
    assert(lambda->data_type->base == popped);
}

static void typecheck_return(ASTNode_T* ret, va_list args)
{
    GET_TYPECHECKER(args);
    ASTType_T* expected = unpack(current_return_type(t));
    char buf1[BUFSIZ] = {'\0'};

    if(!ret->return_val)
    {
        if(expected->kind != TY_VOID)
            throw_error(t->context, ERR_TYPE_ERROR_UNCR, ret->tok, "need to return value of type `%s`", ast_type_to_str(t->context, buf1, expected, LEN(buf1)));
        return;
    }

    Token_T* ret_tok = EITHER(ret->return_val->tok, ret->tok);
    if(!ret->return_val->data_type) {
        throw_error(t->context, ERR_INTERNAL, ret_tok, "no type");
    }

    if(types_equal(t->context, expected, ret->return_val->data_type))
        goto patch_anon_struct;
    
    if(implicitly_castable(t->context,ret_tok, ret->return_val->data_type, expected) == CAST_OK)
    {
        ret->return_val = implicit_cast(t->context, ret_tok, ret->return_val, expected);
        goto patch_anon_struct;
    }

    char buf2[BUFSIZ] = {'\0'};
    throw_error(t->context, ERR_TYPE_ERROR_UNCR, ret_tok, "cannot implicitly cast from `%s` to `%s`", 
        ast_type_to_str(t->context, buf1, ret->return_val->data_type, LEN(buf1)),
        ast_type_to_str(t->context, buf2, expected, LEN(buf2))    
    );

patch_anon_struct:
    if(expected->kind == TY_STRUCT)
        unpack_closure_and_casts(ret->return_val)->data_type = expected;
}

static void typecheck_binop(ASTNode_T* op, va_list args)
{
    GET_TYPECHECKER(args);

    if(types_equal(t->context, op->left->data_type, op->right->data_type))
        return;

    ASTType_T* from = types_equal(t->context, op->data_type, op->right->data_type) ? op->left->data_type : op->right->data_type;
    ASTType_T* to = types_equal(t->context, op->data_type, op->right->data_type) ? op->right->data_type : op->left->data_type;
    assert(from != to && "binop typecheck error"); // should never happen

    ASTNode_T* from_node = from == op->left->data_type ? op->left : op->right;

    char operator = operator_char(op);
    switch(operator)
    {
        case '+':
        case '-':
            if(!(is_numeric(unpack(to)) || is_pointer(unpack(to))) || !(is_numeric(unpack(from)) || is_pointer(unpack(from))))
            {
                throw_error(t->context, ERR_TYPE_ERROR_UNCR, op->tok, "operator `%c` expects integer, float or pointer arguments", operator);
                return;
            }
            break;
        case '%':
            if(!is_integer(unpack(to)) || !is_integer(unpack(from)))
            {
                throw_error(t->context, ERR_TYPE_ERROR_UNCR, op->tok, "operator `%%` expects integer arguments");
                return;
            }
            break;
        case '/':
        case '*':
            if(!is_numeric(unpack(to)) || !is_numeric(unpack(from)))
            {
                throw_error(t->context, ERR_TYPE_ERROR_UNCR, op->tok, "operator `%c` expects integer or float arguments", operator);
                return;
            }
            break;
    }

    if(implicitly_castable(t->context, op->tok, from, to) == CAST_OK)
    {
        ASTNode_T* cast = implicit_cast(t->context, op->tok, from_node, to);
        if(from_node == op->left)
            op->left = cast;
        else if(from_node == op->right)
            op->right = cast;
        else
            unreachable();
        return;
    }

    switch(operator)
    {
        case '+':
            if(is_pointer(unpack(to)) && is_integer(unpack(from)))
                return;
            break;
        case '-':
            if(is_pointer(unpack(to)) && (is_pointer(unpack(from)) || is_integer(unpack(from))))
                return;
            break;
    }

    char buf1[BUFSIZ] = {'\0'};
    char buf2[BUFSIZ] = {'\0'};
    throw_error(t->context, ERR_TYPE_ERROR_UNCR, from_node->tok, "`%c`: cannot implicitly cast from `%s` to `%s`", 
        operator,
        ast_type_to_str(t->context, buf1, from, LEN(buf1)),
        ast_type_to_str(t->context, buf2, to, LEN(buf2))    
    );
}

bool types_equal_strict(Context_T* context, ASTType_T* a, ASTType_T* b)
{
    if(!types_equal(context, a, b))
        return false;
    
    switch(a->kind)
    {
        case TY_STRUCT:
            for(size_t i = 0; i < a->members->size; i++)
            {
                ASTNode_T* am = a->members->items[i];
                ASTNode_T* bm = b->members->items[i];
                if(strcmp(am->id->callee, bm->id->callee) != 0)
                    return false;
            }
            break;

        default:
            break;
    }

    return true;
}

bool types_equal(Context_T* context, ASTType_T* a, ASTType_T* b)
{
    if(!a || !b || a->kind != b->kind || a->is_constant != b->is_constant)
        return false;
    
    switch(a->kind)
    {
        case TY_C_ARRAY:
        case TY_PTR:
            return types_equal(context, a->base, b->base);
        
        case TY_STRUCT:
            if(a->members->size != b->members->size || a->is_union != b->is_union)
                return false;
            for(size_t i = 0; i < a->members->size; i++)
            {
                ASTNode_T* am = a->members->items[i];
                ASTNode_T* bm = b->members->items[i];
                if(!types_equal(context, am->data_type, bm->data_type))
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
                if(!identifiers_equal(am->id, bm->id) || const_i64(context, am->value) != const_i64(context, bm->value))
                    return false;
            }
            return true;

        case TY_UNDEF:
            return identifiers_equal(a->id, b->id);
        
        case TY_FN:
            if(a->arg_types->size != b->arg_types->size || !types_equal(context, a->base, b->base))
                return false;
            for(size_t i = 0; i < a->arg_types->size; i++)
                if(!types_equal(context, a->arg_types->items[i], b->arg_types->items[i]))
                    return false;
            return true;

        default:
            return true;
    }
}

enum IMPLICIT_CAST_RESULT implicitly_castable(Context_T* context, Token_T* tok, ASTType_T* from, ASTType_T* to)
{
    from = unpack(from);
    to = unpack(to);

    if(!from || !to)
        return CAST_ERR;

    if(from->base && to->base && from->base->is_constant && !to->base->is_constant)
        return CAST_DELETING_CONST;
    
    // Buffer for warnings and errors
    char buf1[BUFSIZ] = {'\0'};
    char buf2[BUFSIZ] = {'\0'};

    if(is_integer(from) && is_integer(to))
    {
        //if(from->size > to->size)
        //    throw_error(context, ERR_TYPE_CAST_WARN, tok, "implicitly casting from `%s` to `%s`: possible data loss",
        //        ast_type_to_str(context, buf1, from, LEN(buf1)),
        //        ast_type_to_str(context, buf2, to, LEN(buf2))
        //    );
        return CAST_OK;
    }
    if(is_flonum(from) && is_flonum(to))
        return CAST_OK;
    if(is_integer(from) && is_flonum(to))
        return CAST_OK;
    if(is_flonum(from) && is_integer(to))
    {
        throw_error(context, ERR_TYPE_CAST_WARN, tok, "implicitly casting from `%s` to `%s`",
            ast_type_to_str(context, buf1, from, LEN(buf1)),
            ast_type_to_str(context, buf2, to, LEN(buf2))
        );
        return CAST_OK;
    }
    if((from->kind == TY_PTR || from->kind == TY_C_ARRAY) && to->kind == TY_PTR)
        return CAST_OK;
    if(from->kind == TY_ARRAY && to->kind == TY_VLA)
        return CAST_OK;
    if(from->kind == TY_PTR && unpack(from->base)->kind == TY_ARRAY && to->kind == TY_VLA)
        return types_equal(context, unpack(from->base)->base, to->base) ? CAST_OK : CAST_ERR;
    if(from->kind == TY_STRUCT && to->kind == TY_STRUCT)
        return types_equal(context, from, to) ? CAST_OK : CAST_ERR;
    return CAST_ERR;
}

ASTNode_T* implicit_cast(Context_T* context, Token_T* tok, ASTNode_T* expr, ASTType_T* to)
{
    if(unpack(expr->data_type)->kind == TY_ARRAY && to->kind == TY_VLA)
    {
        ASTNode_T* ref = init_ast_node(&context->raw_allocator, ND_REF, tok);
        ref->data_type = to;
        ref->right = expr;
        return ref;
    }

    ASTNode_T* cast = init_ast_node(&context->raw_allocator, ND_CAST, tok);
    cast->data_type = to;
    cast->left = expr;
    return cast;
}
