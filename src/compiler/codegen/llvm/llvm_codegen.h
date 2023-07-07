#ifdef CSPYDR_USE_LLVM
#ifndef CSPYDR_LLVM_CODEGEN_H
#define CSPYDR_LLVM_CODEGEN_H

#include "ast/ast.h"

void llvm_exit_hook(void);
void generate_llvm(Context_T* context, ASTProg_T *ast, char *output_file, bool print_code, bool is_silent);
i32 llvm_codegen_pass(Context_T* context, ASTProg_T* ast);

#endif
#endif
