#ifndef CSPYDR_AST_VALIDATOR_H
#define CSPYDR_AST_VALIDATOR_H

#include "AST.h"

typedef struct VALIDATOR_STRUCT
{
    list_T* variables;
    list_T* functions;

    AST_T* enclosingAST;

    int numOfGlobalVars;
    int numOfGlobalFns;
} validator_T;

validator_T* initASTValidator();
void validateAST(validator_T* validator, AST_T* ast);

#endif