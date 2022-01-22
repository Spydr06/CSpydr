#include "constexpr.h"
#include "error/error.h"

u64 const_u64(ASTNode_T* node)
{
    switch(node->kind)
    {
        case ND_INT:
            return node->int_val;
        case ND_LONG:
            return node->long_val;
        case ND_LLONG:
            return node->llong_val;
        case ND_SIZEOF:
            return node->the_type->size;
        default:
            throw_error(ERR_CONSTEXPR, node->tok, "`%s` is not a compile-time constant", node->tok->value);
            return 0;
    }
}