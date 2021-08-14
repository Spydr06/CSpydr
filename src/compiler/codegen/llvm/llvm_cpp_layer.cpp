#include "llvm_cpp_layer.hpp"
#include <memory>
#include <iostream>

void free_llvm_cg(LLVMCodegenData_T* cg)
{

}

LLVMCodegenData_T* init_llvm_cg(ASTProg_T* ast)
{
    LLVMCodegenData_T* cg = (LLVMCodegenData_T*) malloc(sizeof(struct LLVM_CODEGEN_DATA_STRUCT));
    cg->print_ll = false;
    cg->silent = false;

    std::unique_ptr<LLVMCodegen> cg_class = std::make_unique<LLVMCodegen>(ast);
    cg->class_callback = cg_class.get();

    return cg;
}

void llvm_gen_code(LLVMCodegenData_T* cg)
{

}

void llvm_emit_code(LLVMCodegenData_T* cg, const char* target)
{

}

void llvm_run_code(LLVMCodegenData_T* cg)
{

}

LLVMCodegen::LLVMCodegen(ASTProg_T* ast)
    : ast(ast) {}