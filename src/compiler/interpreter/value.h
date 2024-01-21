#ifndef CSPYDR_INTERPRETER_VALUE_H
#define CSPYDR_INTERPRETER_VALUE_H

#include <stdint.h>

#include "ast/ast.h"
#include "ast/types.h"
#include "util.h"

#define VOID_VALUE ((InterpreterValue_T){.type=primitives[TY_VOID]})
#define NIL_VALUE  ((InterpreterValue_T){.type=void_ptr_type, .value={.ptr=0}})

#define BOOL_VALUE(v) ((InterpreterValue_T){.type=primitives[TY_BOOL], .value={.boolean=(v)}})
#define TRUE_VALUE    BOOL_VALUE(true)
#define FALSE_VALUE   BOOL_VALUE(false)

#define CHAR_VALUE(v) ((InterpreterValue_T){.type=primitives[TY_CHAR], .value={.character=(v)}})

#define INT_VALUE(csp_type, c_type, v) ((InterpreterValue_T){.type=primitives[(csp_type)], .value={.integer=(InterpreterIntValue_T){.c_type=v}}})
#define I8_VALUE(v)  INT_VALUE(TY_I8, i8, v)
#define I16_VALUE(v) INT_VALUE(TY_I16, i16, v)
#define I32_VALUE(v) INT_VALUE(TY_I32, i32, v)
#define I64_VALUE(v) INT_VALUE(TY_I64, i64, v)

#define UINT_VALUE(csp_type, c_type, v) ((InterpreterValue_T){.type=primitives[(csp_type)], .value={.uinteger=(InterpreterUIntValue_T){.c_type=v}}})
#define U8_VALUE(v)  UINT_VALUE(TY_U8, u8, v)
#define U16_VALUE(v) UINT_VALUE(TY_U16, u16, v)
#define U32_VALUE(v) UINT_VALUE(TY_U32, u32, v)
#define U64_VALUE(v) UINT_VALUE(TY_U64, u64, v)

#define FLOAT_VALUE(csp_type, c_type, v) ((InterpreterValue_T){.type=primitives[(csp_type)], .value={.flt=(InterpreterFloatValue_T){.c_type=v}}})
#define F32_VALUE(v) FLOAT_VALUE(TY_F32, f32, v)
#define F64_VALUE(v) FLOAT_VALUE(TY_F64, f64, v)
#define F80_VALUE(v) FLOAT_VALUE(TY_F80, f80, v)

#define PTR_VALUE(v, ptr_type) ((InterpreterValue_T){.type=(ptr_type), .value={.ptr=(uintptr_t)(v)}})

typedef union INTERPRETER_INT_VALUE_UNION
{
    i8 i8;
    i16 i16;
    i32 i32;
    i64 i64;
} InterpreterIntValue_T;

typedef union INTERPRETER_UINT_VALUE_UNION
{
    u8 u8;
    u16 u16;
    u32 u32;
    u64 u64;
} InterpreterUIntValue_T;

typedef union INTERPRETER_FLOAT_VALUE_UNION
{
    f32 f32;
    f64 f64;
    f80 f80;
} InterpreterFloatValue_T;

typedef union INTERPRETER_VALUE_UNION
{
    bool boolean;
    char character;
    uintptr_t ptr;
    InterpreterIntValue_T integer;
    InterpreterUIntValue_T uinteger;
    InterpreterFloatValue_T flt;
    const ASTObj_T* fn_obj;
} InterpreterValueUnion_T;

typedef struct INTERPRETER_VALUE_STRUCT
{
    const ASTType_T* type;
    InterpreterValueUnion_T value;
} InterpreterValue_T;

void interpreter_value_to_str(InterpreterValue_T* value, char* dst, size_t len);
bool interpreter_value_is_falsy(InterpreterValue_T value);
i64 interpreter_value_i64(InterpreterValue_T* value);
f80 interpreter_value_f80(InterpreterValue_T* value);

#define interpreter_value_is_truthy(value) (!interpreter_value_is_falsy((value)))

#define interpreter_values_equal(a, b) (memcmp(&(a).value, &(b).value, sizeof(InterpreterValueUnion_T)) == 0)

#define ConstInterpreterValueList_M(sz) struct { \
        size_t size;                             \
        size_t allocated;                        \
        InterpreterValue_T data[sz];             \
    }

typedef struct INTERPRETER_VALUE_LIST_STRUCT
{
    size_t size;
    size_t allocated;
    InterpreterValue_T data[];
} InterpreterValueList_T;

#define EMPTY_INTERPERTER_VALUE_LIST ((const InterpreterValueList_T){.size = 0,.allocated = 0})

InterpreterValueList_T* init_interpreter_value_list(size_t capacity);
size_t interpreter_value_list_push(InterpreterValueList_T** list, InterpreterValue_T* value);
void free_interpreter_value_list(InterpreterValueList_T* list);

typedef struct LVALUE_STRUCT 
{
    const ASTType_T* type;
    void* ptr;
} LValue_T;

#endif
