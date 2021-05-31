#ifndef CSPYDR_PRIMITIVES_H
#define CSPYDR_PRIMITIVES_H

#include "ast.h"

#define NUM_TYPES AST_VOID + 1 // AST_VOID is the last item in the ASTDataType_T enum 

// a struct for a single index in the String-to-Type Map
struct StrTypeIdx { 
    char* t;
    ASTDataType_T dt;
};

extern const struct StrTypeIdx str_type_map[NUM_TYPES];
extern ASTType_T* primitives[NUM_TYPES];
extern const int type_byte_size_map[NUM_TYPES];

ASTType_T* get_primitive_type(char* type);

#endif