#ifndef CSPYDR_TRANSPILER_H
#define CSPYDR_TRANSPILER_H

#include "../ast/ast.h"

typedef struct TRANSPILER_STRUCT
{
    const char* target;
    const char* cachePath;

    char* inclSection;
    char* defSection;
    char* implSection;
} transpiler_T;

transpiler_T* initTranspiler(const char* target, const char* cachePath);
void freeTranspiler(transpiler_T* tp);

void transpile(ASTProgram_T* ast, char* target);
void generateCCode(transpiler_T* tp, ASTProgram_T* ast);
void compile(transpiler_T* tp);

#endif