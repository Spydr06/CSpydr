#include "llvm_codegen.h"
#include "llvm_casts.h"

#include <llvm-c/Core.h>
#include <llvm-c/Types.h>

#include <stdio.h>
#include <string.h>

LLVMValueRef llvm_gen_expr(LLVMCodegenData_T* cg, ASTNode_T* node)
{
    switch(node->kind) {
        case ND_INT:
            return LLVMConstInt(LLVMInt32Type(), node->int_val, false);
        case ND_LONG:
            return LLVMConstInt(LLVMInt64Type(), node->long_val, false);
        case ND_LLONG:
            return LLVMConstInt(LLVMInt128Type(), node->llong_val, false);
        case ND_FLOAT:
            return LLVMConstReal(LLVMFloatType(), node->float_val);
        case ND_DOUBLE:
            return LLVMConstReal(LLVMDoubleType(), node->double_val);
        case ND_BOOL:
            return LLVMConstInt(LLVMInt1Type(), node->bool_val, false);
        case ND_STR:
            return LLVMConstString(node->str_val, strlen(node->str_val), false);
        case ND_CHAR:
            return LLVMConstInt(LLVMInt8Type(), strlen(node->str_val) == 1 ? node->str_val[0] : node->str_val[1], false);
        case ND_ID:
            return find_id(cg, llvm_gen_identifier(cg, node->id));
        case ND_CALL:
            return llvm_gen_call(cg, node);
        case ND_ADD:
        case ND_SUB:
        case ND_MUL:
        case ND_DIV:
            return llvm_gen_op(cg, node);
        case ND_NEG:
            return LLVMBuildNeg(cg->llvm_builder, llvm_gen_expr(cg, node->left), "neg");
        case ND_NOT:
            return LLVMBuildNot(cg->llvm_builder, llvm_gen_expr(cg, node->left), "not");
        case ND_BIT_NEG:
            //TODO:
            return NULL;
        case ND_EQ:
        case ND_NE:
        case ND_GT:
        case ND_GE:
        case ND_LT:
        case ND_LE:
            return llvm_gen_cmp(cg, node);
        case ND_CAST:
            return llvm_gen_cast(cg, node);
        case ND_INDEX:
            
        default:
            return NULL;
    }
}

LLVMValueRef llvm_gen_cast(LLVMCodegenData_T* cg, ASTNode_T* cast)
{
    LLVMCastFn cast_fn = llvm_cast_map[cast->left->data_type->kind][cast->data_type->kind];
    if(!cast_fn)
        throw_error(ERR_ILLEGAL_TYPE_CAST, cast->tok, "no cast function for casting from `%s` to `%s` found", cast->left->data_type->tok->value, cast->data_type->tok->value);
        
    return cast_fn(cg, cast);
}

LLVMValueRef llvm_gen_call(LLVMCodegenData_T* cg, ASTNode_T* call)
{
    LLVMValueRef fn = find_fn(cg, llvm_gen_identifier(cg, call->expr->id));
    size_t argc = call->args->size;
    LLVMValueRef* args = argc > 0 ? calloc(argc, sizeof(LLVMValueRef)) : NULL;

    for(size_t i = 0; i < argc; i++)
        args[i] = llvm_gen_expr(cg, call->args->items[i]);

    return LLVMBuildCall(cg->llvm_builder, fn, args, (unsigned) argc, "");
}

LLVMValueRef llvm_gen_op(LLVMCodegenData_T* cg, ASTNode_T* op)
{
    LLVMValueRef left =  llvm_gen_expr(cg, op->left);
    LLVMValueRef right = llvm_gen_expr(cg, op->right);

    switch(op->kind)
    {
        case ND_ADD:
            return LLVMBuildBinOp(cg->llvm_builder, LLVMAdd, left, right, "add");
        case ND_SUB:
            return LLVMBuildBinOp(cg->llvm_builder, LLVMSub, left, right, "sub");
        case ND_MUL:
            return LLVMBuildBinOp(cg->llvm_builder, LLVMMul, left, right, "mul");
        case ND_DIV:
            return LLVMBuildBinOp(cg->llvm_builder, LLVMFDiv, left, right, "fdiv");
        default:
            throw_error(ERR_UNDEFINED, op->tok, "undefined op type %d", op->kind);
    }

    return NULL;
}

LLVMValueRef llvm_gen_cmp(LLVMCodegenData_T* cg, ASTNode_T* op)
{
    //TODO:

    return NULL;
}