#include "value.h"
#include "ast/ast.h"
#include "ast/types.h"
#include "codegen/codegen_utils.h"
#include "error/error.h"
#include "io/log.h"
#include "lexer/token.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

void interpreter_value_to_str(InterpreterValue_T* value, char* dst, size_t len)
{
    switch(value->type->kind)
    {
        case TY_I8:
        case TY_I16:
        case TY_I32:
        case TY_I64:
            snprintf(dst, len, "%ld", value->value.integer.i64);
            break;
        case TY_U8:
        case TY_U16:
        case TY_U32:
        case TY_U64:
            snprintf(dst, len, "%lu", value->value.uinteger.u64);
            break;
    
        case TY_F32:
            snprintf(dst, len, "%f", value->value.flt.f32);
            break;
        
        case TY_F64:
            snprintf(dst, len, "%lf", value->value.flt.f64);
            break;
        
        case TY_F80:
            snprintf(dst, len, "%Lf", value->value.flt.f80);
            break;
        
        case TY_VOID:
            strncpy(dst, "<void>", len);
            break;

        case TY_BOOL:
            strncpy(dst, value->value.boolean ? "true" : "false", len);
            break;
        
        case TY_CHAR:
            snprintf(dst, len, "%d", value->value.character);
            break;

        case TY_PTR:
            if(value->value.ptr == 0)
                strncpy(dst, "<nil>", len);
            else
                snprintf(dst, len, "%#lx", value->value.ptr);
            break;

        default:
            strncpy(dst, "<unimplemented value>", len);
            break;
    } 
}

bool interpreter_value_is_falsy(InterpreterValue_T value)
{
    switch(value.type->kind)
    {
        case TY_I8:
        case TY_I16:
        case TY_I32:
        case TY_I64:
            return value.value.integer.i64 == 0;

        case TY_U8:
        case TY_U16:
        case TY_U32:
        case TY_U64:
            return value.value.uinteger.u64 == 0;

        case TY_F32:
            return value.value.flt.f32 == 0;

        case TY_F64:
            return value.value.flt.f64 == 0;
        
        case TY_F80:
            return value.value.flt.f80 == 0;
        
        case TY_VOID:
            return true;
        
        case TY_BOOL:
            return !value.value.boolean;
        
        case TY_CHAR:
            return value.value.character == '\0';
        
        case TY_PTR:
            return value.value.ptr == 0;
        
        default:
            assert(false);
            return true;
    }
}

i64 interpreter_value_i64(InterpreterValue_T* value)
{
    switch(value->type->kind)
    {
        case TY_I8:
        case TY_I16:
        case TY_I32:
        case TY_I64:
            return value->value.integer.i64;

        case TY_U8:
        case TY_U16:
        case TY_U32:
        case TY_U64:
            return (i64) value->value.uinteger.u64;

        case TY_PTR:
            return (i64) value->value.ptr;

        case TY_CHAR:
            return (i64) value->value.character;

        default:
            unreachable();
            return 0;
    }
}

f80 interpreter_value_f80(InterpreterValue_T* value)
{
    switch(value->type->kind)
    {
        case TY_F32:
            return (f80) value->value.flt.f32;

        case TY_F64:
            return (f80) value->value.flt.f64;
        
        case TY_F80:
            return value->value.flt.f80;
        
        default:
            unreachable();
            return 0.0L;
    }
}

ASTNode_T* ast_node_from_interpreter_value(Context_T* context, InterpreterValue_T* value, Token_T* tok)
{
    ASTNode_T* node = init_ast_node(ND_INT, tok);
    node->data_type = (ASTType_T*) value->type;

    switch(value->type->kind)
    {
        case TY_I8:
            node->int_val = value->value.integer.i8;
            break;
        case TY_I16:
            node->int_val = value->value.integer.i16;
            break;
        case TY_I32:
            node->int_val = value->value.integer.i32;
            break;
        case TY_I64:
            node->kind = ND_LONG;
            node->long_val = value->value.integer.i64;
            break;
        case TY_U8:
            node->int_val = value->value.uinteger.u8;
            break;
        case TY_U16:
            node->int_val = value->value.uinteger.u16;
            break;
        case TY_U32:
            node->kind = ND_LONG;
            node->long_val = value->value.uinteger.u32;
            break;
        case TY_U64:
            node->kind = ND_ULONG;
            node->ulong_val = value->value.uinteger.u64;
            break;
        case TY_F32:
            node->kind = ND_FLOAT;
            node->float_val = value->value.flt.f32;
            break;
        case TY_F64:
            node->kind = ND_DOUBLE;
            node->double_val = value->value.flt.f64;
            break;
        case TY_F80: // TODO: hande precision correctly
            node->kind = ND_DOUBLE;
            node->double_val = value->value.flt.f80;
            break;
        case TY_BOOL:
            node->kind = ND_BOOL;
            node->bool_val = value->value.boolean;
            break;
        case TY_CHAR:
            node->kind = ND_CHAR;
            node->int_val = value->value.character;
            break;
        case TY_FN:
            node->kind = ND_ID;
            node->id = value->value.fn_obj->id;
            node->referenced_obj = (ASTObj_T*) value->value.fn_obj;
            break;
        case TY_PTR:
            if(unpack(node->data_type->base)->kind == TY_CHAR)
            {
                node->kind = ND_CHAR;
                node->str_val = strdup((const char*) value->value.ptr);
                break;
            }

            node->kind = ND_ULONG;
            node->ulong_val = (u64) value->value.ptr;
            break;
        default: 
        {
            char buf[BUFSIZ] = {'\0'};
            throw_error(context, ERR_CONSTEXPR, tok, "invalid constexpr return type `%s`", 
                ast_type_to_str(context, buf, value->type, BUFSIZ)
            );
        }
    }

    return node;
}

InterpreterValueList_T* init_interpreter_value_list(size_t capacity)
{
    InterpreterValueList_T* list = malloc(capacity * sizeof(InterpreterValue_T) + sizeof(InterpreterValueList_T));
    list->size = 0;
    list->allocated = capacity;
    return list;
}

size_t interpreter_value_list_push(InterpreterValueList_T** list, InterpreterValue_T* value)
{
    // cannot push to constant lists
    assert((*list)->allocated);

    if((*list))
    {
        (*list)->allocated *= 2;
        *list = realloc(*list, (*list)->allocated * sizeof(InterpreterValue_T) + sizeof(InterpreterValueList_T)); // TODO: find better allocation curve
    }

    size_t pos = (*list)->size;
    (*list)->size++;
    memcpy(&(*list)->data[pos], value, sizeof(InterpreterValue_T));

    return pos;
}

void free_interpreter_value_list(InterpreterValueList_T* list)
{
    if(list->allocated != 0)
        free(list);
}
