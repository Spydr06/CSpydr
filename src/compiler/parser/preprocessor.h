#ifndef CSPYDR_OPTIMIZER_H
#define CSPYDR_OPTIMIZER_H

#include "../list.h"
#include "../error/errorHandler.h"
#include "../ast/ast.h"

// this is the stage, where the AST gets optimized and all expression types get resolved

typedef struct PREPROCESSOR_STRUCT {
    list_T* typedefs;
    list_T* functions; 
    list_T* locals;
    list_T* globals;
    list_T* args;

    errorHandler_T* eh;
    int errors;
} preprocessor_T;

preprocessor_T* initPreprocessor(errorHandler_T* eh);
void freePreprocessor(preprocessor_T* pre);

void optimizeAST(preprocessor_T* opt, ASTProgram_T* ast);

#endif