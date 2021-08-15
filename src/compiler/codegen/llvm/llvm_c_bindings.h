#ifndef CSPYDR_C_BINDINGS
#define CSPYDR_C_BINDINGS

#include "../../ast/ast.h"

#ifdef __cplusplus
    #define __CSP_CXX_FN extern "C"
#else
    #define __CSP_CXX_FN extern
#endif

__CSP_CXX_FN void llvm_gen_code(ASTProg_T* prog, bool silent, bool print_llvm);
__CSP_CXX_FN void llvm_emit_code(const char* target, bool silent);
__CSP_CXX_FN void llvm_run_code(bool silent);

#endif