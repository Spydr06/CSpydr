#ifndef CSPYDR_BYTECODE_COMPILER_H
#define CSPYDR_BYTECODE_COMPILER_H

#include "../core/parser/AST.h"
#include "../core/list.h"
#include "instructions.h"

typedef struct BYTECODE_COMPILER_STRUCT
{
    AST_T* root;
    list_T* instructions;
} BCCompiler_T;

BCCompiler_T* initBCCompiler();
void compileBC(BCCompiler_T* compiler, AST_T* root);

#endif
