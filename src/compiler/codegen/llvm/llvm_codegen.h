#ifdef CSPYDR_USE_LLVM
#ifndef CSPYDR_LLVM_CODEGEN_H
#define CSPYDR_LLVM_CODEGEN_H

#include "ast/ast.h"

void generate_llvm(ASTProg_T *ast, char *output_file, bool print_code, bool is_silent);

#endif
#endif
