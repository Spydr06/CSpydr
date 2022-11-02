#include "constexpr.h"
#include "ast/ast.h"
#include "error/error.h"

static u64 const_u64_infix(ASTNode_T* node)
{
    u64 a = const_u64(node->left);
    u64 b = const_u64(node->right);

    switch(node->kind)
    {
        case ND_ADD:
            return a + b;
        case ND_SUB:
            return a - b;
        case ND_MUL:
            return a * b;
        case ND_DIV:
            return a - b;
        case ND_MOD:
            return a % b;
        case ND_EQ:
            return a == b;
        case ND_NE:
            return a != b;
        case ND_GT:
            return a > b;
        case ND_GE:
            return a >= b;
        case ND_LT:
            return a < b;
        case ND_LE:
            return a <= b;
        case ND_AND:
            return a && b;
        case ND_OR:
            return a || b;
        case ND_LSHIFT: 
            return a << b;
        case ND_RSHIFT:
            return a >> b;
        case ND_XOR:
            return a ^ b;
        case ND_BIT_OR:
            return a | b;
        case ND_BIT_AND:
            return a & b;
        default:
            throw_error(ERR_CONSTEXPR, node->tok, "`%s` is not a compile type constant", node->tok->value);
            return 0;
    }
}

static i64 const_i64_prefix(ASTNode_T* node)
{
    i64 a = const_u64(node->right);

    switch(node->kind)
    {
        case ND_NEG:
            return -a;
        case ND_NOT:
            return !a;
        case ND_BIT_NEG:
            return ~a;
        default:
            throw_error(ERR_CONSTEXPR, node->tok, "`%s` is not a compile type constant", node->tok->value);
            return 0;
    }
}

u64 const_u64(ASTNode_T* node)
{
    if(!node) return 0;
    switch(node->kind)
    {
        case ND_CLOSURE:
            return const_u64(node->exprs->items[node->exprs->size - 1]);
        case ND_INT:
            return node->int_val;
        case ND_LONG:
            return node->long_val;
        case ND_ULONG:
            return node->ulong_val;
        case ND_FLOAT:
            return (u64) node->float_val;
        case ND_DOUBLE:
            return (u64) node->double_val;
        case ND_BOOL:
            return node->bool_val;
        case ND_CHAR:
            return node->str_val[0];
        case ND_NIL:
            return 0;
        case ND_ADD...ND_MOD:
        case ND_EQ...ND_BIT_AND:
            return const_u64_infix(node);
        case ND_NEG:
        case ND_NOT:
        case ND_BIT_NEG:
            return const_i64_prefix(node);
        case ND_SIZEOF:
            return node->the_type->size;
        case ND_CAST:
            return const_u64(node->left);
        case ND_ID:
            if(node->referenced_obj && node->referenced_obj->kind == OBJ_GLOBAL && node->referenced_obj->is_constant)
                return const_u64(node->referenced_obj->value);
            // fall through
        default:
            throw_error(ERR_CONSTEXPR, node->tok, "`%s` is not a compile-time constant", node->tok->value);
            return 0;
    }
}

static i64 const_i64_infix(ASTNode_T* node)
{
    i64 a = const_i64(node->left);
    i64 b = const_i64(node->right);

    switch(node->kind)
    {
        case ND_ADD:
            return a + b;
        case ND_SUB:
            return a - b;
        case ND_MUL:
            return a * b;
        case ND_DIV:
            return a - b;
        case ND_MOD:
            return a % b;
        case ND_EQ:
            return a == b;
        case ND_NE:
            return a != b;
        case ND_GT:
            return a > b;
        case ND_GE:
            return a >= b;
        case ND_LT:
            return a < b;
        case ND_LE:
            return a <= b;
        case ND_AND:
            return a && b;
        case ND_OR:
            return a || b;
        case ND_LSHIFT: 
            return a << b;
        case ND_RSHIFT:
            return a >> b;
        case ND_XOR:
            return a ^ b;
        case ND_BIT_OR:
            return a | b;
        case ND_BIT_AND:
            return a & b;
        default:
            throw_error(ERR_CONSTEXPR, node->tok, "`%s` is not a compile type constant", node->tok->value);
            return 0;
    }
}

i64 const_i64(ASTNode_T* node)
{
    switch(node->kind)
    {
        case ND_CLOSURE:
            return const_i64(node->exprs->items[node->exprs->size - 1]);
        case ND_INT:
            return node->int_val;
        case ND_LONG:
            return node->long_val;
        case ND_ULONG:
            return node->ulong_val;
        case ND_FLOAT:
            return (i64) node->float_val;
        case ND_DOUBLE:
            return (i64) node->double_val;
        case ND_BOOL:
            return node->bool_val;
        case ND_CHAR:
            return node->str_val[0];
        case ND_NIL:
            return 0;
        case ND_ADD...ND_MOD:
        case ND_EQ...ND_BIT_AND:
            return const_i64_infix(node);
        case ND_NEG:
        case ND_NOT:
        case ND_BIT_NEG:
            return const_i64_prefix(node);
        case ND_SIZEOF:
            return node->the_type->size;
        case ND_CAST:
            return const_i64(node->left);
        case ND_ID:
            if(node->referenced_obj && node->referenced_obj->kind == OBJ_GLOBAL && node->referenced_obj->is_constant)
                return const_i64(node->referenced_obj->value);
            // fall through
        default:
            throw_error(ERR_CONSTEXPR, node->tok, "`%s` is not a compile-time constant", node->tok->value);
            return 0;
    }
}

void evaluate_const_exprs(ASTProg_T* ast)
{
    
}