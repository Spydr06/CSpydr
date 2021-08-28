#include "llvm_codegen.h"

#include <llvm-c/Analysis.h>
#include <string.h>

LLVMValueRef llvm_gen_obj(LLVMCodegenData_T* cg, ASTObj_T* obj)
{
    switch(obj->kind)
    {
        case OBJ_GLOBAL:
            return llvm_gen_global(cg, obj);
            break;
        case OBJ_LOCAL:
            break;
        case OBJ_FUNCTION:
            return llvm_gen_fn(cg, obj);
            break;
        case OBJ_FN_ARG:
            break;
        case OBJ_TYPEDEF:
            break;
    }
    return NULL;
}

LLVMValueRef llvm_gen_fn(LLVMCodegenData_T* cg, ASTObj_T* obj)
{
    LLVMTypeRef return_type = llvm_gen_type(cg, obj->return_type);
    size_t argc = obj->args->size;

    LLVMTypeRef* arg_types = calloc(argc, sizeof(LLVMTypeRef));
    for(size_t i = 0; i < argc; i++)
    {
        arg_types[i] = llvm_gen_type(cg, ((ASTObj_T*) obj->args->items[i])->data_type);
    }   

    LLVMTypeRef fn_type = LLVMFunctionType(return_type, arg_types, argc, false);
    LLVMValueRef fn = LLVMAddFunction(cg->llvm_module, obj->callee, fn_type);

    cg->current_fn = fn;
    cg->current_fn_ast = obj;
    list_push(cg->fns, fn);

    if(strcmp(obj->callee, "main") == 0)
        cg->main_fn = obj;

    free(arg_types);
    return fn;
}

void llvm_gen_fn_body(LLVMCodegenData_T* cg, ASTObj_T* fn)
{
    LLVMBasicBlockRef prev_block = llvm_begin_label(cg, "entry");
    llvm_gen_stmt(cg, fn->body);
    llvm_end_label(cg, prev_block);

    LLVMVerifyFunction(cg->current_fn, LLVMPrintMessageAction);
}

LLVMValueRef llvm_gen_global(LLVMCodegenData_T* cg, ASTObj_T* ast)
{
    LLVMTypeRef type = llvm_gen_type(cg, ast->data_type);
    LLVMValueRef global = LLVMAddGlobal(cg->llvm_module, type, ast->callee);

    list_push(cg->vars, global);

    return global;
}