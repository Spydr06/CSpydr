#ifndef CSPYDR_PRIMITIVES_H
#define CSPYDR_PRIMITIVES_H

#include "ast.h"

#define NUM_TYPES AST_VOID + 1 // AST_VOID is the last item in the ASTDataType_T enum 

extern ASTType_T* primitives[NUM_TYPES];

struct StrTypeIdx { 
    char* t;
    ASTDataType_T dt;
};

extern const struct StrTypeIdx strTypeMap[NUM_TYPES];

ASTType_T* getPrimitiveType(char* type);

#endif