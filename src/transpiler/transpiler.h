#ifndef CSPYDR_TRANSPILER_H
#define CSPYDR_TRANSPILER_H

#include "../core/parser/AST.h"

typedef struct TRANSPILER_STRUCT
{
    char* includeSection;
    char* defineSection;
    char* codeSection;

    char* stdPath;
} transpiler_T;

transpiler_T* initTranspiler(char* stdPath);

void transpileAST(ASTRoot_T* ast, transpiler_T* transpiler);

char* emitCode(transpiler_T* transpiler);

#endif