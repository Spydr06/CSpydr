#include "types.h"
#include "ast.h"
#include "memory/allocator.h"

#include <string.h>
#include <stdbool.h>
#include <assert.h>

ASTNode_T* constant_literals[TOKEN_EOF] = { // sets value literals, who are always the same to save memory
    [TOKEN_TRUE]  = &(ASTNode_T){.kind = ND_BOOL, .bool_val = true,  .is_constant = true},
    [TOKEN_FALSE] = &(ASTNode_T){.kind = ND_BOOL, .bool_val = false, .is_constant = true},
    [TOKEN_NIL]   = &(ASTNode_T){.kind = ND_NIL, .int_val = 0,       .is_constant = true},
};

const ASTType_T* primitives[NUM_TYPES] = {    // sets the primitive data types, who are always the same to save memory
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
    [TY_C_ARRAY] = 0,    // TODO: evaluate further
    [TY_ENUM]   = ENUM_S,
    [TY_STRUCT] = 0
};

const ASTType_T* char_ptr_type = &(ASTType_T)
{
    .kind = TY_PTR,
    .size = PTR_S, 
    .base = &(ASTType_T)
    {
        .kind = TY_CHAR,
        .is_primitive = true,
        .size = CHAR_S,
    }
};

const ASTType_T* void_ptr_type = &(ASTType_T)
{
    .kind = TY_PTR,
    .size = PTR_S,
    .base = &(ASTType_T)
    {
        .kind = TY_VOID,
        .is_primitive = true,
        .size = VOID_S
    }
};

#define U32_LIT &(ASTType_T){.kind = TY_U32, .size = U32_S, .is_primitive = true}
#define VOIDPTR_LIT &(ASTType_T){.kind = TY_PTR, .size = PTR_S, .base = &(ASTType_T){.kind = TY_VOID, .is_primitive = true, .size = VOID_S}}

ASTTypeKind_T get_datatype_from_str(char* str)
{
    for(int i = 0; i < NUM_TYPES; i++)
        if(str_type_map[i].t)
            if(strcmp(str_type_map[i].t, str) == 0)
                return str_type_map[i].dt;
    return TY_UNDEF;
}

ASTType_T* get_primitive_type(Allocator_T* allocator, char* type)
{
    ASTTypeKind_T kind = get_datatype_from_str(type);

    const ASTType_T* prim = primitives[kind];

    if(prim) 
    {
        ASTType_T* dupl = init_ast_type(allocator, prim->kind, prim->tok);
        dupl->is_primitive = prim->is_primitive;
        dupl->size = prim->size;
        return dupl;
    }

    return NULL;
}

ASTType_T* ptr_to(Allocator_T* allocator, Token_T* tok, ASTType_T* base)
{
    ASTType_T* ptr = init_ast_type(allocator, TY_PTR, tok);

    ptr->base = base;
    ptr->size = PTR_S;

    return ptr;
}

bool is_signed_integer_type(ASTType_T* ty)
{
     return ty && (
        ty->kind == TY_I8 || 
        ty->kind == TY_I16 || 
        ty->kind == TY_I32 || 
        ty->kind == TY_I64 ||
        ty->kind == TY_ENUM 
    );
}

bool is_unsigned_integer_type(ASTType_T* ty)
{
    return ty && (
        ty->kind == TY_U8 || 
        ty->kind == TY_U16 || 
        ty->kind == TY_U32 || 
        ty->kind == TY_U64
    );
}
