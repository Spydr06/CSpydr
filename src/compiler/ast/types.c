#include "types.h"

#include <string.h>

ASTType_T* primitives[NUM_TYPES] = {
    [AST_I8]  = &(ASTType_T){.type = AST_I8,  .isPrimitive = true},
    [AST_I16] = &(ASTType_T){.type = AST_I16, .isPrimitive = true},
    [AST_I32] = &(ASTType_T){.type = AST_I32, .isPrimitive = true},
    [AST_I64] = &(ASTType_T){.type = AST_I64, .isPrimitive = true},

    [AST_U8]  = &(ASTType_T){.type = AST_U8,  .isPrimitive = true},
    [AST_U16] = &(ASTType_T){.type = AST_U16, .isPrimitive = true},
    [AST_U32] = &(ASTType_T){.type = AST_U32, .isPrimitive = true},
    [AST_U64] = &(ASTType_T){.type = AST_U64, .isPrimitive = true},

    [AST_F32] = &(ASTType_T){.type = AST_F32, .isPrimitive = true},
    [AST_F64] = &(ASTType_T){.type = AST_F64, .isPrimitive = true},
    [AST_F80] = &(ASTType_T){.type = AST_F80, .isPrimitive = true},

    [AST_VOID] = &(ASTType_T){.type = AST_VOID, .isPrimitive = true},
    [AST_CHAR] = &(ASTType_T){.type = AST_CHAR, .isPrimitive = true},
    [AST_BOOL] = &(ASTType_T){.type = AST_BOOL, .isPrimitive = true}
};

const struct StrTypeIdx strTypeMap[NUM_TYPES] = {
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

ASTDataType_T getDataTypeFromStr(char* str)
{
    for(int i = 0; i < NUM_TYPES; i++)
        if(strTypeMap[i].t)
            if(strcmp(strTypeMap[i].t, str) == 0)
                return strTypeMap[i].dt;
    return AST_TYPEDEF;
}

ASTType_T* getPrimitiveType(char* type)
{
    ASTType_T* prim = primitives[getDataTypeFromStr(type)];
    return prim;
}