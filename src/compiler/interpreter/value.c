#include "value.h"

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
    // cannot free constant lists
    assert(list->allocated);
    free(list);
}
