#ifndef CSPYDR_OPTIMIZER_H
#define CSPYDR_OPTIMIZER_H

#include "../list.h"
#include "../error/errorHandler.h"
#include "../ast/ast.h"

// this is the stage, where the AST gets optimized and all expression types get resolved

typedef struct PREPROCESSOR_STRUCT {
    List_T* typedefs;
    List_T* functions; 
    List_T* locals;
    List_T* globals;
    List_T* args;

    ErrorHandler_T* eh;
    int errors;
} Preprocessor_T;

Preprocessor_T* init_preprocessor(ErrorHandler_T* eh);
void free_preprocessor(Preprocessor_T* pre);

void optimize_ast(Preprocessor_T* opt, ASTProgram_T* ast);

#endif