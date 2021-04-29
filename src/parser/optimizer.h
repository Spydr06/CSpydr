#ifndef CSPYDR_OPTIMIZER_H
#define CSPYDR_OPTIMIZER_H

#include "../list.h"
#include "../error/errorHandler.h"
#include "../ast/ast.h"

// this is the stage, where the AST gets optimized and all expression types get resolved

typedef struct OPTIMIZER_STRUCT {
    list_T* imports;
    list_T* typedefs;
    list_T* functions; 
    list_T* locals;
    list_T* globals;

    errorHandler_T* eh;
} optimizer_T;

optimizer_T* initOptimizer();
void freeOptimizer(optimizer_T* opt);

void optimizeAST(optimizer_T* opt, ASTProgram_T* ast);

#endif