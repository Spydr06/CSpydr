#include "constexpr.h"

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
    }
}