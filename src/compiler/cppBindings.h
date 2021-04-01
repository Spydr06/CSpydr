#ifndef CSPYDR_LLVM_H
#define CSPYDR_LLVM_H

#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "../core/parser/AST.h"

void compile(AST_T* ast, const char* targetPath, const char* sourcePath);

#endif