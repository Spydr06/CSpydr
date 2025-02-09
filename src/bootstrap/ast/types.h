#ifndef CSPYDR_AST_TYPES_H
#define CSPYDR_AST_TYPES_H

#include "ast.h"
#include "memory/allocator.h"

#define NUM_TYPES TY_UNDEF + 1 // TY_UNDEF is the last item in the ASTDataType_T enum. TY_UNDEF should never occur

#define I8_S   1
#define I16_S  2
#define I32_S  4
#define I64_S  8
#define U8_S   1
#define U16_S  2
#define U32_S  4
#define U64_S  8
#define F32_S  4
#define F64_S  8
#define F80_S  10
#define CHAR_S 1
#define BOOL_S 1
#define VOID_S 1
#define PTR_S  8
#define ENUM_S 4
#define DYN_S (PTR_S * 2) // vtable ptr + data ptr

// a struct for a single index in the String-to-Type Map
struct StrTypeIdx { 
    char* t;
    ASTTypeKind_T dt;
};
extern const struct StrTypeIdx str_type_map[NUM_TYPES];

extern const ASTType_T* primitives[NUM_TYPES];

extern const int type_byte_size_map[NUM_TYPES];

extern const ASTType_T* char_ptr_type;
extern const ASTType_T* void_ptr_type;

ASTTypeKind_T get_datatype_from_str(char* str);
ASTType_T* get_primitive_type(Allocator_T* alloc, char* type);
bool check_type_compatibility(ASTType_T* a, ASTType_T* b);
ASTType_T* ptr_to(Allocator_T* alloc, Token_T* tok, ASTType_T* base);

bool is_prototype(ASTType_T* ty);
bool is_signed_integer_type(ASTType_T* ty);
bool is_unsigned_integer_type(ASTType_T* ty);

#endif
