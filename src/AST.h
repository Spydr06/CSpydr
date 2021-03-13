#ifndef CSPYDR_AST_H
#define CSPYDR_AST_H

#include "list.h"

typedef enum AST_TYPE
{
    AST_ROOT,
    AST_FUNCTION,
    AST_VARIABLE,
    AST_STATEMENT,
    AST_EXPRESSION,
    AST_COMPOUND,
    AST_INT,
    AST_STRING,
    AST_CALL,
    AST_ASSIGNMENT,
    AST_NOOP,
} ASTType_T;

typedef struct AST_STRUCT
{
    ASTType_T type;

    char* name;
    list_T* children;
    struct AST_STRUCT* value;
    int intValue;
    char* dataType;
    char* strValue;
} AST_T;

AST_T* initAST(int type);

#endif