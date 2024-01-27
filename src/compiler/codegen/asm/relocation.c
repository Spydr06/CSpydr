#include "relocation.h"
#include "ast/ast.h"
#include "codegen/asm/asm_codegen.h"
#include "codegen/codegen_utils.h"
#include "ast/types.h"
#include "optimizer/constexpr.h"
#include "error/error.h"

#include <stdint.h>
#include <string.h>

static size_t asm_gen_const_data(ASMCodegenData_T* cg, size_t size)
{
    if(!size)
        return 0;
    if(size > 8)
        unreachable();
    
    switch(size)
    {
        case U8_S:
            asm_print(cg, "  .byte ");
            break;
        case U16_S:
            asm_print(cg, "  .2byte ");
            break;
        case U32_S:
            asm_print(cg, "  .4byte ");
            break;
        case U64_S:
            asm_print(cg, "  .8byte ");
            break;
        default:
            unreachable();
            return 0;
    }
    return size;
}

u8 escape_sequence(char c, const char* str, size_t* i) 
{
    switch(c) 
    {
        case 'a':
            return '\a';
        case 'b':
            return '\b';
        case 't':
            return '\t';
        case 'v':
            return '\v';
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case 'f':
            return '\f';
        case 'e':
            return '\e';
        case '"':
        case '\'':
        case '\\':
            return c;
        case 'x':
            *i += 2;
            return strtol((char[3]){str[1], str[2], '\0'}, NULL, 16);
        case '0':
            return '\0';
        default:
            return c;
    }
}

size_t gen_relocation(ASMCodegenData_T* cg, ASTNode_T* node, size_t target_size)
{
    if(!node)
        return 0;
    
    size_t size_generated = 0;
    //memset(buffer, 0, target_size);
    
    switch(node->kind) 
    {
        case ND_NIL:
            size_generated = asm_gen_const_data(cg, target_size);
            asm_println(cg, "0");
            break;

        case ND_STR:
            {
                size_t size = MIN(strlen(node->str_val), target_size - 1);
                for(size_t i = 0; i < size; i++)
                {
                    u8 value = node->str_val[i] == '\\' ? (i++, escape_sequence(node->str_val[i], &node->str_val[i], &i)) : node->str_val[i];
                    asm_println(cg, "  .byte %u", value);
                }
                asm_println(cg, "  .byte 0");
                if(size_generated < target_size)
                    asm_println(cg, "  .zero %zu", target_size - size_generated);
            } break;

        case ND_CHAR:
            size_generated = asm_gen_const_data(cg, target_size);
            asm_println(cg, "%u", node->int_val);
            break;

        case ND_BOOL:
            size_generated = asm_gen_const_data(cg, target_size);
            asm_println(cg, "%u", node->bool_val);
            break;

        case ND_INT:
            size_generated = asm_gen_const_data(cg, target_size);
            asm_println(cg, "%u", node->int_val);
            break;

        case ND_LONG:
            size_generated = asm_gen_const_data(cg, target_size);
            asm_println(cg, "%zu", node->long_val);
            break;

        case ND_ULONG:
            size_generated = asm_gen_const_data(cg, target_size);
            asm_println(cg, "%zu", node->long_val);
            break;

        case ND_FLOAT:
        {
            union {
                f32 in;
                uint32_t out;
            } conv = {.in = node->float_val};

            size_generated = asm_gen_const_data(cg, target_size);
            asm_println(cg, "%u", conv.out);
        } break;

        case ND_DOUBLE: 
        {
            union {
                f64 in;
                uint64_t out;
            } conv = {.in = node->double_val};

            size_generated = asm_gen_const_data(cg, target_size);
            asm_println(cg, "%zu", conv.out);
        } break;

        case ND_ARRAY:
        {
            size_generated = asm_gen_const_data(cg, PTR_S);
            asm_println(cg, "%zu", node->data_type->num_indices);
            size_t index_size = node->data_type->base->size;
            for(size_t i = 0; i < node->args->size; i++)
                size_generated += gen_relocation(cg, node->args->items[i], index_size);
        } break;

        case ND_STRUCT:
        {
            ASTType_T* struct_type = unpack(node->data_type);
            for(size_t i = 0; i < node->args->size; i++)
            {
                ASTNode_T* arg = node->args->items[i];
                size_t offset = ((ASTNode_T*) struct_type->members->items[i])->offset;

                if(size_generated < offset)
                    asm_println(cg, "  .zero %zu", offset - size_generated); // pad correctly
                size_generated += gen_relocation(cg, arg, arg->data_type->size);
            }  
        } break;
        
        case ND_CLOSURE:
            size_generated = gen_relocation(cg, node->expr, target_size);
            break;
        
        case ND_NEG:
            node->right->int_val = -node->right->int_val; // temporary
            size_generated = gen_relocation(cg, node->right, target_size);
            break;

        case ND_CAST:
            size_generated = gen_relocation(cg, node->left, node->data_type->size);
            break;
        
        case ND_ID:
            if(target_size <= 8)
            {
                size_generated = asm_gen_const_data(cg, target_size);
                asm_println(cg, "%s", asm_gen_identifier(cg->context, node->id));
                break;
            }
            throw_error(cg->context, ERR_CODEGEN, node->tok, "cannot generate relocation for identifiers with types > 8 bytes");
            break;

        default:
            if(is_unsigned_integer_type(node->data_type))
            {
                size_generated = asm_gen_const_data(cg, target_size);
                asm_println(cg, "%zu", const_u64(cg->context, node));
                return size_generated;
            }
            else if(is_signed_integer_type(node->data_type))
            {
                i64 raw_data = const_i64(cg->context, node);
                size_generated = asm_gen_const_data(cg, target_size);
                switch(node->data_type->size) {
                case I64_S:
                    asm_println(cg, "%zu", raw_data);
                    break;
                case I32_S: {
                    asm_println(cg, "%u", (i32) raw_data);
                    break;
                }
                case I16_S: {
                    asm_println(cg, "%u", (i16) raw_data);
                    break;
                }
                case I8_S: {
                    asm_println(cg, "%u", (i8) raw_data);
                    break;
                }
                default:
                    unreachable();
                }
                return size_generated;
            }

            throw_error(cg->context, ERR_CODEGEN, node->tok, "cannot generate relocation for `%s` (%d)", node->tok->value, node->kind);
            break;
    }

    if(size_generated < target_size)
        asm_println(cg, "  .zero %zu", target_size - size_generated);

    return size_generated;
}
