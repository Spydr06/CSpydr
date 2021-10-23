#include "types.h"
#include "ast.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

ASTNode_T* constant_literals[TOKEN_EOF] = { // sets value literals, who are always the same to save memory
    [TOKEN_TRUE]  = &(ASTNode_T){.kind = ND_BOOL, .bool_val = true,  .is_constant = true},
    [TOKEN_FALSE] = &(ASTNode_T){.kind = ND_BOOL, .bool_val = false, .is_constant = true},
    [TOKEN_NIL]   = &(ASTNode_T){.kind = ND_NIL, .int_val = 0,       .is_constant = true},
};

ASTType_T* primitives[NUM_TYPES] = {    // sets the primitive data types, who are always the same to save memory
    [TY_I8]  = &(ASTType_T){.kind = TY_I8,    .is_primitive = true, .size = I8_S},
    [TY_I16] = &(ASTType_T){.kind = TY_I16,   .is_primitive = true, .size = I16_S},
    [TY_I32] = &(ASTType_T){.kind = TY_I32,   .is_primitive = true, .size = I32_S},
    [TY_I64] = &(ASTType_T){.kind = TY_I64,   .is_primitive = true, .size = I64_S},

    [TY_U8]  = &(ASTType_T){.kind = TY_U8,    .is_primitive = true, .size = U8_S},
    [TY_U16] = &(ASTType_T){.kind = TY_U16,   .is_primitive = true, .size = U16_S},
    [TY_U32] = &(ASTType_T){.kind = TY_U32,   .is_primitive = true, .size = U32_S},
    [TY_U64] = &(ASTType_T){.kind = TY_U64,   .is_primitive = true, .size = U64_S},

    [TY_F32] = &(ASTType_T){.kind = TY_F32,   .is_primitive = true, .size = F32_S},
    [TY_F64] = &(ASTType_T){.kind = TY_F64,   .is_primitive = true, .size = F64_S},
    [TY_F80] = &(ASTType_T){.kind = TY_F80,   .is_primitive = true, .size = F80_S},

    [TY_VOID] = &(ASTType_T){.kind = TY_VOID, .is_primitive = true, .size = VOID_S},
    [TY_CHAR] = &(ASTType_T){.kind = TY_CHAR, .is_primitive = true, .size = CHAR_S},
    [TY_BOOL] = &(ASTType_T){.kind = TY_BOOL, .is_primitive = true, .size = BOOL_S},
    
    [TY_FN] = &(ASTType_T){.kind = TY_FN, .is_primitive = true},
    [TY_VA_LIST] = &(ASTType_T){.kind = TY_VA_LIST, .is_primitive = true},
};

const struct StrTypeIdx str_type_map[NUM_TYPES] = { // a lookup-chart to find the corresponding data-types from a string
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
};

const int type_byte_size_map[NUM_TYPES] = { // a array to find the size in bytes of primitive data types
    [TY_I8]  = I8_S,
    [TY_I16] = I16_S,
    [TY_I32] = I32_S,
    [TY_I64] = I64_S,

    [TY_U8]  = U8_S,
    [TY_U16] = U16_S,
    [TY_U32] = U32_S,
    [TY_U64] = U64_S,

    [TY_F32] = F32_S,
    [TY_F64] = F64_S,
    [TY_F80] = F80_S,

    [TY_CHAR] = CHAR_S,
    [TY_BOOL] = BOOL_S,
    [TY_VOID] = VOID_S,

    [TY_PTR] = PTR_S,
    [TY_ARR] = 0,    // TODO: evaluate further
    [TY_ENUM]   = ENUM_S,
    [TY_STRUCT] = 0
};

const bool type_compatibility_map[NUM_TYPES][NUM_TYPES] = {
    //             i8, i16, i32, i64, u8, u16, u32, u64, f32, f64, f80, bool, void, char, ptr, array, struct, opaque struct, enum, lambda, tuple, template, undef
    [TY_I8]     = {1,  1,   1,   1,   1,  1,   1,   1,   1,   1,   1,   1,    0,    1,    0,   0,     0,      0,             1,     0,     0,     1,        1},
    [TY_I16]    = {1,  1,   1,   1,   1,  1,   1,   1,   1,   1,   1,   1,    0,    0,    0,   0,     0,      0,             1,     0,     0,     1,        1},
    [TY_I32]    = {1,  1,   1,   1,   1,  1,   1,   1,   1,   1,   1,   1,    0,    0,    0,   0,     0,      0,             1,     0,     0,     1,        1},
    [TY_I64]    = {1,  1,   1,   1,   1,  1,   1,   1,   1,   1,   1,   1,    0,    0,    0,   0,     0,      0,             1,     0,     0,     1,        1},

    [TY_U8]     = {1,  1,   1,   1,   1,  1,   1,   1,   1,   1,   1,   1,    0,    1,    0,   0,     0,      0,             1,     0,     0,     1,        1},
    [TY_U16]    = {1,  1,   1,   1,   1,  1,   1,   1,   1,   1,   1,   1,    0,    0,    0,   0,     0,      0,             1,     0,     0,     1,        1},
    [TY_U32]    = {1,  1,   1,   1,   1,  1,   1,   1,   1,   1,   1,   1,    0,    0,    0,   0,     0,      0,             1,     0,     0,     1,        1},
    [TY_U64]    = {1,  1,   1,   1,   1,  1,   1,   1,   1,   1,   1,   1,    0,    0,    1,   0,     0,      0,             1,     0,     0,     1,        1},

    [TY_F32]    = {1,  1,   1,   1,   1,  1,   1,   1,   1,   1,   1,   1,    0,    0,    0,   0,     0,      0,             0,     0,     0,     1,        1},
    [TY_F64]    = {1,  1,   1,   1,   1,  1,   1,   1,   1,   1,   1,   1,    0,    0,    0,   0,     0,      0,             0,     0,     0,     1,        1},
    [TY_F80]    = {1,  1,   1,   1,   1,  1,   1,   1,   1,   1,   1,   1,    0,    0,    0,   0,     0,      0,             0,     0,     0,     1,        1},

    [TY_BOOL]   = {1,  1,   1,   1,   1,  1,   1,   1,   1,   1,   1,   1,    0,    1,    0,   0,     0,      0,             1,     0,     0,     1,        1},
    [TY_VOID]   = {0,  0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,    1,    0,    0,   0,     0,      0,             0,     0,     0,     1,        1},
    [TY_CHAR]   = {1,  1,   1,   1,   1,  1,   1,   1,   1,   1,   1,   1,    0,    1,    0,   0,     0,      0,             1,     0,     0,     1,        1},
    [TY_PTR]    = {0,  0,   0,   0,   0,  0,   0,   1,   0,   0,   0,   0,    0,    0,    1,   1,     0,      0,             0,     0,     0,     1,        1},
    [TY_ARR]    = {0,  0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,    0,    0,    1,   1,     0,      0,             0,     0,     0,     1,        1},
    [TY_STRUCT] = {0,  0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,    0,    0,    0,   0,     1,      1,             0,     0,     1,     1,        1},
    [TY_ENUM]   = {1,  1,   1,   1,   1,  1,   1,   1,   1,   1,   1,   1,    0,    1,    0,   0,     0,      0,             1,     0,     0,     1,        1},
    [TY_LAMBDA] = {0,  0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,    0,    0,    0,   0,     0,      0,             0,     1,     0,     1,        1},
    [TY_TUPLE]  = {0,  0,   0,   0,   0,  0,   0,   0,   0,   0,   1,   0,    0,    0,    0,   0,     1,      0,             0,     0,     1,     1,        1},
    [TY_TEMPLATE] = {1,1,   1,   1,   1,  1,   1,   1,   1,   1,   1,   1,    1,    1,    1,   1,     1,      0,             1,     1,     1,     1,        1},
    [TY_UNDEF]  = {1,  1,   1,   1,   1,  1,   1,   1,   1,   1,   1,   1,    1,    1,    1,   1,     1,      0,             1,     1,     1,     1,        1},
    [TY_OPAQUE_STRUCT] = {0,0,0, 0,   0,  0,   0,   0,   0,   0,   0,   0,    0,    0,    0,   0,     1,      1,             0,     0,     0,     0,        0},
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

    if(prim) 
    {
        ASTType_T* dupl = init_ast_type(prim->kind, prim->tok);
        dupl->is_primitive = prim->is_primitive;
        dupl->size = prim->size;
        return dupl;
    }

    return NULL;
}

bool check_type_compatibility(ASTType_T* a, ASTType_T* b)
{
    return type_compatibility_map[a->kind][b->kind];
}