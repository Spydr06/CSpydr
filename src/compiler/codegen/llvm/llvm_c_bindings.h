#ifndef CSPYDR_C_BINDINGS
#define CSPYDR_C_BINDINGS

#include "../../ast/ast.h"

typedef struct LLVM_CODEGEN_DATA_STRUCT
{
    void* class_callback; // for the c++ class

    bool silent;
    bool print_ll;
} LLVMCodegenData_T;

#ifdef __cplusplus
    #define __CSP_CPP_FN extern "C"
#else
    #define __CSP_CPP_FN extern
#endif

__CSP_CPP_FN void free_llvm_cg(LLVMCodegenData_T*);
__CSP_CPP_FN LLVMCodegenData_T* init_llvm_cg(ASTProg_T*);

__CSP_CPP_FN void llvm_gen_code(LLVMCodegenData_T*);
__CSP_CPP_FN void llvm_emit_code(LLVMCodegenData_T*, const char*);
__CSP_CPP_FN void llvm_run_code(LLVMCodegenData_T*);

#endif