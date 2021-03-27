#ifndef CSPYDR_LLVM_COMPILER_H
#define CSPYDR_LLVM_COMPILER_H

#include <llvm-c/Types.h>
#include <stdbool.h>
#include "llvm.h"
#include "../core/AST.h"

typedef struct LLVM_COMPILER_STRUCT
{
    LLVMModuleRef root;
} LLVMCompiler_T;

typedef struct LLVM__VALUE_TYPE
{
    enum {
        INT, FLOAT, BOOLEAN, STRING, POINTER,
    } kind;

    LLVMTypeRef type;
    bool isSigned;
} valueType_T;

LLVMCompiler_T initCompiler();
void compileProgram(AST_T* root, char* outputFile, char* srcFile);

valueType_T* initValueType(LLVMTypeRef type, bool isSigned, int kind);
valueType_T* dataTypeToLLVMTypeRef(AST_T* ast);

#endif