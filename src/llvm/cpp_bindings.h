#ifndef CSPYDR_CPP_BINDINGS_H
#define CSPYDR_CPP_BINDINGS_H

#include "../ast/ast.h"

extern void generateLLVM(ASTProgram_T* ast, const char* targetPath, const char* sourcePath);

#endif