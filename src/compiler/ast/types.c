#include "types.h"
#include "ast.h"

#include <string.h>
#include <stdbool.h>

ASTType_T* primitives[NUM_TYPES] = {
    [AST_I8]  = &(ASTType_T){.type = AST_I8,  .is_primitive = true},
    [AST_I16] = &(ASTType_T){.type = AST_I16, .is_primitive = true},
    [AST_I32] = &(ASTType_T){.type = AST_I32, .is_primitive = true},
    [AST_I64] = &(ASTType_T){.type = AST_I64, .is_primitive = true},

    [AST_U8]  = &(ASTType_T){.type = AST_U8,  .is_primitive = true},
    [AST_U16] = &(ASTType_T){.type = AST_U16, .is_primitive = true},
    [AST_U32] = &(ASTType_T){.type = AST_U32, .is_primitive = true},
    [AST_U64] = &(ASTType_T){.type = AST_U64, .is_primitive = true},

    [AST_F32] = &(ASTType_T){.type = AST_F32, .is_primitive = true},
    [AST_F64] = &(ASTType_T){.type = AST_F64, .is_primitive = true},
    [AST_F80] = &(ASTType_T){.type = AST_F80, .is_primitive = true},

    [AST_VOID] = &(ASTType_T){.type = AST_VOID, .is_primitive = true},
    [AST_CHAR] = &(ASTType_T){.type = AST_CHAR, .is_primitive = true},
    [AST_BOOL] = &(ASTType_T){.type = AST_BOOL, .is_primitive = true}
};

const struct StrTypeIdx str_type_map[NUM_TYPES] = {
    {"i8",  AST_I8},
    {"i16", AST_I16},
    {"i32", AST_I32},
    {"i64", AST_I64},  

    {"u8",  AST_U8},
    {"u16", AST_U16},
    {"u32", AST_U32},
    {"u64", AST_U64},

    {"f32", AST_F32},
    {"f64", AST_F64},
    {"f80", AST_F80},

    {"void", AST_VOID},
    {"char", AST_CHAR},
    {"bool", AST_BOOL},
    
    //non-primitives (still useful)
    {"struct", AST_STRUCT},
    {"enum", AST_ENUM},
    {"*", AST_POINTER},
    {"[", AST_ARRAY},
    {NULL, AST_TYPEDEF}
};

const int type_byte_size_map[NUM_TYPES] = {
    [AST_I8]  = 1,
    [AST_I16] = 2,
    [AST_I32] = 4,
    [AST_I64] = 8,

    [AST_U8]  = 1,
    [AST_U16] = 2,
    [AST_U32] = 4,
    [AST_U64] = 8,

    [AST_F32] = 4,
    [AST_F64] = 8,
    [AST_F80] = 16,

    [AST_CHAR] = 1,
    [AST_BOOL] = 1,
    [AST_VOID] = 1,

    [AST_POINTER] = 8,
    [AST_ARRAY]   = 0,    // TODO: evaluate further
    [AST_ENUM]    = 4,
    [AST_STRUCT]  = 0
};

ASTDataType_T get_datatype_from_str(char* str)
{
    for(int i = 0; i < NUM_TYPES; i++)
        if(str_type_map[i].t)
            if(strcmp(str_type_map[i].t, str) == 0)
                return str_type_map[i].dt;
    return AST_TYPEDEF;
}

ASTType_T* get_primitive_type(char* type)
{
    ASTType_T* prim = primitives[get_datatype_from_str(type)];
    return prim;
}