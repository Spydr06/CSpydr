#ifndef CSPYDR_TYPES_H
#define CSPYDR_TYPES_H

#include "ast.h"

#define NUM_TYPES TY_UNDEF + 1 // TY_UNDEF is the last item in the ASTDataType_T enum. TY_UNDEF should never occur

// a struct for a single index in the String-to-Type Map
struct StrTypeIdx { 
    char* t;
    ASTTypeKind_T dt;
};

extern const struct StrTypeIdx str_type_map[NUM_TYPES];
extern ASTType_T* primitives[NUM_TYPES];
extern const int type_byte_size_map[NUM_TYPES];

ASTType_T* get_primitive_type(char* type);

#endif