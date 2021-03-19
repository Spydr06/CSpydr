#ifndef CSPYDR_LLVM_COMPILER_H
#define CSPYDR_LLVM_COMPILER_H

#include <stdbool.h>
#include "llvm.h"
#include "../core/AST.h"

typedef struct LLVM_COMPILER_STRUCT
{
    LLVMModuleRef root;
} LLVMCompiler_T;

LLVMCompiler_T initCompiler();
void compileProgram(AST_T* root, char* outputFile);

LLVMTypeRef dataTypeToLLVMTypeRef(AST_T* ast);

#endif