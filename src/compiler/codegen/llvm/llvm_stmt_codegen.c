#include "llvm_codegen.h"
#include "../../ast/types.h"

void llvm_gen_stmt(LLVMCodegenData_T* cg, ASTNode_T* node)
{
    switch(node->kind)
    {
        case ND_BLOCK:
            llvm_gen_block(cg, node);
            break;
        case ND_RETURN:
            llvm_gen_return(cg, node);
            break;
        case ND_IF:
            llvm_gen_if(cg, node);
            break;
        case ND_LOOP:
            llvm_gen_loop(cg, node);
            break;
        default:
            break;
    }
}

LLVMBasicBlockRef llvm_begin_label(LLVMCodegenData_T* cg, char* name)
{
    LLVMBasicBlockRef prev_block = cg->current_block;

    LLVMBasicBlockRef block = LLVMAppendBasicBlock(cg->current_fn, name);
    LLVMPositionBuilderAtEnd(cg->llvm_builder, block);

    size_t argc = cg->current_fn_ast->args->size;
    if(argc > 0)
    {
        LLVMValueRef* arg_allocs = calloc(argc, sizeof(LLVMValueRef));
        LLVMValueRef* arg_values = calloc(argc, sizeof(LLVMValueRef));
        LLVMGetParams(cg->current_fn, arg_values);

        for(size_t i = 0; i < argc; i++)
        {
            ASTObj_T* arg = cg->current_fn_ast->args->items[i];
            LLVMSetValueName(arg_values[i], arg->callee);
            arg_allocs[i] = LLVMBuildAlloca(cg->llvm_builder, llvm_gen_type(cg, arg->data_type), "");
            LLVMBuildStore(cg->llvm_builder, arg_values[i], arg_allocs[i]);
            list_push(cg->vars, arg_values[i]);
        }
    }

    return prev_block;
}

void llvm_end_label(LLVMCodegenData_T* cg, LLVMBasicBlockRef prev_block)
{
    size_t argc = cg->current_fn_ast->args->size;
    for(size_t i = 0; i < argc; i++)
        cg->vars->size--;

    LLVMPositionBuilderAtEnd(cg->llvm_builder, prev_block);
    cg->current_block = prev_block;
}

void llvm_gen_block(LLVMCodegenData_T* cg, ASTNode_T* node)
{   
    for(size_t i = 0; i < node->locals->size; i++)
        llvm_gen_local(cg, node->locals->items[i]);

    for(size_t i = 0; i < node->stmts->size; i++)
        llvm_gen_stmt(cg, node->stmts->items[i]);

    if(cg->current_fn_ast->return_type == primitives[TY_VOID])
        LLVMBuildRetVoid(cg->llvm_builder);
    
    for(size_t i = 0; i < node->locals->size; i++)
        cg->vars->size--;
}

void llvm_gen_return(LLVMCodegenData_T* cg, ASTNode_T* node)
{
    if(node->return_val)
    {
        LLVMValueRef val = llvm_gen_expr(cg, node->return_val);
        LLVMBuildRet(cg->llvm_builder, val);
    } else
        LLVMBuildRetVoid(cg->llvm_builder);
}

void llvm_gen_if(LLVMCodegenData_T* cg, ASTNode_T* node)
{
    LLVMBasicBlockRef true_block = LLVMAppendBasicBlock(cg->current_fn, "if.then");
    LLVMBasicBlockRef false_block = node->else_branch ? LLVMAppendBasicBlock(cg->current_fn, "if.else") : LLVMAppendBasicBlock(cg->current_fn, "if.else");

    LLVMValueRef cond = llvm_gen_expr(cg, node->condition);
    LLVMBuildCondBr(cg->llvm_builder, cond, true_block, false_block); 

    LLVMPositionBuilderAtEnd(cg->llvm_builder, true_block);
    llvm_gen_stmt(cg, node->if_branch);

    if(node->else_branch)
    {
        LLVMBuildBr(cg->llvm_builder, true_block);
        LLVMPositionBuilderAtEnd(cg->llvm_builder, false_block);
        llvm_gen_stmt(cg, node->else_branch);
    }
}

void llvm_gen_local(LLVMCodegenData_T* cg, ASTObj_T* local)
{
   //TODO:
}

void llvm_gen_loop(LLVMCodegenData_T* cg, ASTNode_T* node)
{

}