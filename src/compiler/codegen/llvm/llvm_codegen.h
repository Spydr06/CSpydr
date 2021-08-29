#ifndef CSPYDR_LLVM_CODEGEN_H
#define CSPYDR_LLVM_CODEGEN_H

#include "../../ast/ast.h"
#include "../../error/error.h"

#include <llvm-c/Core.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Types.h>

typedef struct LLVM_CODEGEN_DATA_STRUCT
{
    ASTProg_T* ast;
    bool silent;

    LLVMModuleRef  llvm_module;
    LLVMBuilderRef llvm_builder;

    LLVMValueRef      current_fn;
    ASTObj_T*         current_fn_ast;
    LLVMBasicBlockRef current_block;

    ASTObj_T* main_fn;

    List_T* vars;
    List_T* fns;
} LLVMCodegenData_T;

LLVMCodegenData_T* init_llvm_cg(ASTProg_T* ast);
void               free_llvm_cg(LLVMCodegenData_T* cg);

void llvm_gen_code(LLVMCodegenData_T* cg);
void llvm_print_code(LLVMCodegenData_T* cg);

void llvm_emit_code(LLVMCodegenData_T* cg, const char* target);
void llvm_run_code(LLVMCodegenData_T* cg);

void llvm_exit_hook(void);

/*********************************
 **      Internal functions     **
 ********************************/

// llvm_codegen.c
LLVMValueRef find_id(LLVMCodegenData_T* cg, char* callee);
LLVMValueRef find_fn(LLVMCodegenData_T* cg, char* callee);

// llvm_type_codegen.c
LLVMTypeRef llvm_gen_type(LLVMCodegenData_T* cg, ASTType_T* ty);

// llvm_obj_codegen.c
LLVMValueRef llvm_gen_obj(LLVMCodegenData_T* cg, ASTObj_T* obj);
LLVMValueRef llvm_gen_fn(LLVMCodegenData_T* cg, ASTObj_T* obj);
void llvm_gen_fn_body(LLVMCodegenData_T* cg, ASTObj_T* fn);
LLVMValueRef llvm_gen_global(LLVMCodegenData_T* cg, ASTObj_T* ast);

// llvm_stmt_codegen.c
void llvm_gen_stmt(LLVMCodegenData_T* cg, ASTNode_T* node);
LLVMBasicBlockRef llvm_begin_label(LLVMCodegenData_T* cg, char* name);
void llvm_end_label(LLVMCodegenData_T* cg, LLVMBasicBlockRef prev_block);
void llvm_gen_block(LLVMCodegenData_T* cg, ASTNode_T* node);
void llvm_gen_return(LLVMCodegenData_T* cg, ASTNode_T* node);
void llvm_gen_if(LLVMCodegenData_T* cg, ASTNode_T* node);
void llvm_gen_local(LLVMCodegenData_T* cg, ASTObj_T* local);
void llvm_gen_loop(LLVMCodegenData_T* cg, ASTNode_T* node);

// llvm_expr_codegen.c
LLVMValueRef llvm_gen_expr(LLVMCodegenData_T* cg, ASTNode_T* stmt);
LLVMValueRef llvm_gen_cast(LLVMCodegenData_T* cg, ASTNode_T* cast);
LLVMValueRef llvm_gen_call(LLVMCodegenData_T* cg, ASTNode_T* call);
LLVMValueRef llvm_gen_op(LLVMCodegenData_T* cg, ASTNode_T* op);
LLVMValueRef llvm_gen_cmp(LLVMCodegenData_T* cg, ASTNode_T* op);

#endif