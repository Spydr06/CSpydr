#ifndef CSPYDR_TRANSPILER_H
#define CSPYDR_TRANSPILER_H

#include "../core/AST.h"

typedef struct TRANSPILER_STRUCT
{
    char* includeSection;
    char* defineSection;
    char* codeSection;
} transpiler_T;

transpiler_T* initTranspiler();
void writeToInclude(transpiler_T* transpiler ,char* str);
void writeToDefine(transpiler_T* transpiler, char* str);
void writeToCode(transpiler_T* transpiler, char* str);

char* transpilerGetCode(transpiler_T* transpiler);

char* transpileToC(transpiler_T* transpiler, AST_T* root);

#endif