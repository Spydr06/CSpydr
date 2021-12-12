#ifndef CSPYDR_TYPES_H
#define CSPYDR_TYPES_H

#include "ast.h"

#define NUM_TYPES TY_UNDEF + 1 // TY_UNDEF is the last item in the ASTDataType_T enum. TY_UNDEF should never occur

#define I8_S   sizeof(signed char)
#define I16_S  sizeof(signed short)
#define I32_S  sizeof(signed int)
#define I64_S  sizeof(signed long)
#define U8_S   sizeof(unsigned char)
#define U16_S  sizeof(unsigned short)
#define U32_S  sizeof(unsigned int)
#define U64_S  sizeof(unsigned long)
#define F32_S  sizeof(float)
#define F64_S  sizeof(double)
#define F80_S  sizeof(long double)
#define CHAR_S sizeof(char)
#define BOOL_S sizeof(bool)
#define VOID_S sizeof(void)
#define PTR_S  sizeof(void*)
#define ENUM_S sizeof(int)

// a struct for a single index in the String-to-Type Map
struct StrTypeIdx { 
    char* t;
    ASTTypeKind_T dt;
};
extern const struct StrTypeIdx str_type_map[NUM_TYPES];

extern const ASTType_T* primitives[NUM_TYPES];

extern const int type_byte_size_map[NUM_TYPES];
extern ASTNode_T* constant_literals[TOKEN_EOF];

extern const bool type_cast_map[NUM_TYPES][NUM_TYPES];
extern const ASTType_T* char_ptr_type;
extern const ASTType_T* void_ptr_type;

ASTTypeKind_T get_datatype_from_str(char* str);
ASTType_T* get_primitive_type(char* type);
bool check_type_compatibility(ASTType_T* a, ASTType_T* b);

#endif