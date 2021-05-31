#include "types.h"
#include "ast.h"

#include <string.h>
#include <stdbool.h>

ASTType_T* primitives[NUM_TYPES] = {
    [TY_I8]  = &(ASTType_T){.kind = TY_I8,  .is_primitive = true},
    [TY_I16] = &(ASTType_T){.kind = TY_I16, .is_primitive = true},
    [TY_I32] = &(ASTType_T){.kind = TY_I32, .is_primitive = true},
    [TY_I64] = &(ASTType_T){.kind = TY_I64, .is_primitive = true},

    [TY_U8]  = &(ASTType_T){.kind = TY_U8,  .is_primitive = true},
    [TY_U16] = &(ASTType_T){.kind = TY_U16, .is_primitive = true},
    [TY_U32] = &(ASTType_T){.kind = TY_U32, .is_primitive = true},
    [TY_U64] = &(ASTType_T){.kind = TY_U64, .is_primitive = true},

    [TY_F32] = &(ASTType_T){.kind = TY_F32, .is_primitive = true},
    [TY_F64] = &(ASTType_T){.kind = TY_F64, .is_primitive = true},
    [TY_F80] = &(ASTType_T){.kind = TY_F80, .is_primitive = true},

    [TY_VOID] = &(ASTType_T){.kind = TY_VOID, .is_primitive = true},
    [TY_CHAR] = &(ASTType_T){.kind = TY_CHAR, .is_primitive = true},
    [TY_BOOL] = &(ASTType_T){.kind = TY_BOOL, .is_primitive = true}
};

const struct StrTypeIdx str_type_map[NUM_TYPES] = {
    {"i8",  TY_I8},
    {"i16", TY_I16},
    {"i32", TY_I32},
    {"i64", TY_I64},  

    {"u8",  TY_U8},
    {"u16", TY_U16},
    {"u32", TY_U32},
    {"u64", TY_U64},

    {"f32", TY_F32},
    {"f64", TY_F64},
    {"f80", TY_F80},

    {"void", TY_VOID},
    {"char", TY_CHAR},
    {"bool", TY_BOOL},
    
    //non-primitives (still useful)
    {"struct", TY_STRUCT},
    {"enum", TY_ENUM},
    {"*", TY_PTR},
    {"[", TY_ARR},
    {NULL, TY_UNDEF}
};

const int type_byte_size_map[NUM_TYPES] = {
    [TY_I8]  = 1,
    [TY_I16] = 2,
    [TY_I32] = 4,
    [TY_I64] = 8,

    [TY_U8]  = 1,
    [TY_U16] = 2,
    [TY_U32] = 4,
    [TY_U64] = 8,

    [TY_F32] = 4,
    [TY_F64] = 8,
    [TY_F80] = 16,

    [TY_CHAR] = 1,
    [TY_BOOL] = 1,
    [TY_VOID] = 1,

    [TY_PTR] = 8,
    [TY_ARR]   = 0,    // TODO: evaluate further
    [TY_ENUM]    = 4,
    [TY_STRUCT]  = 0
};

ASTTypeKind_T get_datatype_from_str(char* str)
{
    for(int i = 0; i < NUM_TYPES; i++)
        if(str_type_map[i].t)
            if(strcmp(str_type_map[i].t, str) == 0)
                return str_type_map[i].dt;
    return TY_UNDEF;
}

ASTType_T* get_primitive_type(char* type)
{
    ASTType_T* prim = primitives[get_datatype_from_str(type)];
    return prim;
}