#include "relocation.h"
#include "codegen/codegen_utils.h"
#include "ast/types.h"
#include "optimizer/constexpr.h"
#include "error/error.h"
#include "io/log.h"

#include <string.h>

void gen_relocation(ASTNode_T* node, size_t target_size, u8* buffer)
{
    if(!node)
        return;
    
    //memset(buffer, 0, target_size);
    
    switch(node->kind) 
    {
        case ND_NIL:
            break;

        case ND_STR:
            {
                size_t size = MIN(strlen(node->str_val), target_size - 1);
                memcpy(buffer, node->str_val, size);
            } break;

        case ND_CHAR:
            buffer[0] = (char) node->int_val;   
            break;

        case ND_BOOL:
            buffer[0] = node->bool_val;
            break;

        case ND_INT:
            memcpy(buffer, &node->int_val, MIN(U64_S, target_size));
            break;

        case ND_LONG:
            memcpy(buffer, &node->long_val, MIN(U64_S, target_size));
            break;

        case ND_ULONG:
            memcpy(buffer, &node->ulong_val, MIN(U64_S, target_size));
            break;

        case ND_FLOAT:
            memcpy(buffer, &node->float_val, MIN(F32_S, target_size));
            break;

        case ND_DOUBLE:
            memcpy(buffer, &node->double_val, MIN(F64_S, target_size));
            break;

        case ND_ARRAY:
        {
            size_t index_size = node->data_type->base->size;
            memcpy(buffer, &node->data_type->num_indices, PTR_S);
            for(size_t i = 0; i < node->args->size; i++)
                gen_relocation(node->args->items[i], index_size, buffer + index_size * i + PTR_S);
        } break;

        case ND_STRUCT:
            LOG_ERROR("Not implemented\n");
            break;
        
        case ND_CLOSURE:
            gen_relocation(node->expr, target_size, buffer);
            break;
        
        case ND_NEG:
            node->right->int_val = -node->right->int_val; // temporary
            gen_relocation(node->right, target_size, buffer);
            break;

        case ND_CAST:
            gen_relocation(node->left, node->data_type->size, buffer);
            break;
        
        case ND_ID:
            if(node->referenced_obj && node->referenced_obj->value)
            {
                gen_relocation(node->referenced_obj->value, target_size, buffer);
                break;
            }

        default:
            throw_error(ERR_CODEGEN, node->tok, "cannot generate relocation for `%s` (%d)", node->tok->value, node->kind);
            break;
    }
}