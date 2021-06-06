#ifndef CSPYDR_LLVM_CODEGEN_H
#define CSPYDR_LLVM_CODEGEN_H

#include "../../ast/ast.h"

#include <llvm-c/Core.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Types.h>

typedef struct LLVM_CODEGEN_DATA_STRUCT
{
    ASTProg_T* ast;
    bool print_ll;
    bool silent;

    LLVMModuleRef  llvm_module;
    LLVMBuilderRef llvm_builder;

    LLVMValueRef      current_fn;
    ASTObj_T*         current_fn_ast;
    LLVMBasicBlockRef current_block;

    ASTObj_T* main_fn;

    List_T* vars;
} LLVMCodegenData_T;

LLVMCodegenData_T* init_llvm_cg(ASTProg_T* ast);
void               free_llvm_cg(LLVMCodegenData_T* cg);

void llvm_gen_code(LLVMCodegenData_T* cg);

void llvm_emit_code(LLVMCodegenData_T* cg, const char* target);
void llvm_run_code(LLVMCodegenData_T* cg);

#endif