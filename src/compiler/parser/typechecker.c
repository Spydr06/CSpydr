#include "typechecker.h"

#include <stdio.h>
#include <string.h>

#include "ast/ast.h"
#include "error/error.h"
#include "optimizer/constexpr.h"
#include "parser/validator.h"
#include "utils.h"
#include "codegen/codegen_utils.h"

bool types_equal(ASTType_T* a, ASTType_T* b)
{
    if(!a || !b || a->kind != b->kind || a->is_constant != b->is_constant)
        return false;
    
    switch(a->kind)
    {
        case TY_ARR:
        case TY_PTR:
            return types_equal(a->base, b->base);
        
        case TY_STRUCT:
            if(a->members->size != b->members->size || a->is_union != b->is_union)
                return false;
            for(size_t i = 0; i < a->members->size; i++)
            {
                ASTNode_T* am = a->members->items[i];
                ASTNode_T* bm = b->members->items[i];
                if(!identifiers_equal(am->id, bm->id) || !types_equal(am->data_type, bm->data_type))
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

void typecheck_assignment(Validator_T* v, ASTNode_T* assignment)
{
    if(types_equal(assignment->left->data_type, assignment->right->data_type))
        return;

    if(implicitly_castable(v, assignment->tok, assignment->right->data_type, assignment->left->data_type))
    {
        assignment->right = implicit_cast(assignment->tok, assignment->right, assignment->left->data_type);
        return;
    }

    char buf1[BUFSIZ] = {};
    char buf2[BUFSIZ] = {};
    throw_error(ERR_TYPE_ERROR_UNCR, assignment->tok, "assignment type missmatch: cannot assign `%s` to `%s`", 
        ast_type_to_str(buf1, assignment->right->data_type, LEN(buf1)),
        ast_type_to_str(buf2, assignment->left->data_type, LEN(buf2))    
    );
}

ASTNode_T* typecheck_arg_pass(Validator_T* v, ASTType_T* expected, ASTNode_T* received)
{
    if(types_equal(expected, received->data_type))
        return received;
    
    if(implicitly_castable(v, received->tok, received->data_type, expected))
        return implicit_cast(received->tok, received, expected);
    
    char buf1[BUFSIZ] = {};
    char buf2[BUFSIZ] = {};
    throw_error(ERR_TYPE_ERROR_UNCR, received->tok, "cannot implicitly cast from `%s` to `%s`", 
        ast_type_to_str(buf1, received->data_type, LEN(buf1)),
        ast_type_to_str(buf2, expected, LEN(buf2))    
    );
}

bool implicitly_castable(Validator_T* v, Token_T* tok, ASTType_T* from, ASTType_T* to)
{
    from = expand_typedef(v, from);
    to = expand_typedef(v, to);

    if(!from || !to)
        return false;
    
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
        return true;
    }
    if(is_flonum(from) && is_flonum(to))
        return true;
    if(is_integer(from) && is_flonum(to))
        return true;
    if(is_flonum(from) && is_integer(to))
    {
        throw_error(ERR_TYPE_CAST_WARN, tok, "implicitly casting from `%s` to `%s`",
            ast_type_to_str(buf1, from, LEN(buf1)),
            ast_type_to_str(buf2, to, LEN(buf2))
        );
        return true;
    }
    if((from->kind == TY_PTR || from->kind == TY_ARR) && to->kind == TY_PTR)
        return true;
    return false;
}

ASTNode_T* implicit_cast(Token_T* tok, ASTNode_T* expr, ASTType_T* to)
{
    ASTNode_T* cast = init_ast_node(ND_CAST, tok);
    cast->data_type = to;
    cast->the_type = expr->data_type;
    cast->left = expr;
    return cast;
}