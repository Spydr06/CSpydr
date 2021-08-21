#include "llvm_casts.h"
#include "llvm_codegen.h"
#include <llvm-c/Core.h>
#include <llvm-c/Types.h>

static LLVMValueRef skip(LLVMCodegenData_T* cg, ASTNode_T* cast);

static LLVMValueRef itoi(LLVMCodegenData_T* cg, ASTNode_T* cast);
static LLVMValueRef ftof(LLVMCodegenData_T* cg, ASTNode_T* cast);
static LLVMValueRef ftoi(LLVMCodegenData_T* cg, ASTNode_T* cast);
static LLVMValueRef ftou(LLVMCodegenData_T* cg, ASTNode_T* cast);
static LLVMValueRef itof(LLVMCodegenData_T* cg, ASTNode_T* cast);
static LLVMValueRef utof(LLVMCodegenData_T* cg, ASTNode_T* cast);
static LLVMValueRef utou(LLVMCodegenData_T* cg, ASTNode_T* cast);
static LLVMValueRef utoi(LLVMCodegenData_T* cg, ASTNode_T* cast);
static LLVMValueRef itou(LLVMCodegenData_T* cg, ASTNode_T* cast);

LLVMCastFn llvm_cast_map[NUM_TYPES][NUM_TYPES] = {
    // type        i8,      i16,    i32,    i64,    u8,     u16,    u32,    u64,    f32,    f64,    bool,   char,   void,   ptr,    arr,    enum,   struct
    [TY_I8]     = {skip,    itoi,   itoi,   itoi,   itou,   itou,   itou,   itou,   itof,   itof,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL},
    [TY_I16]    = {itoi,    skip,   itoi,   itoi,   itou,   itou,   itou,   itou,   itof,   itof,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL},
    [TY_I32]    = {itoi,    itoi,   skip,   itoi,   itou,   itou,   itou,   itou,   itof,   itof,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL},
    [TY_I64]    = {itoi,    itoi,   itoi,   skip,   itou,   itou,   itou,   itou,   itof,   itof,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL},
    [TY_U8]     = {utoi,    utoi,   utoi,   utoi,   skip,   utou,   utou,   utou,   utof,   utof,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL},
    [TY_U16]    = {utoi,    utoi,   utoi,   utoi,   utou,   skip,   utou,   utou,   utof,   utof,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL},
    [TY_U32]    = {utoi,    utoi,   utoi,   utoi,   utou,   utou,   skip,   utou,   utof,   utof,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL},
    [TY_U64]    = {utoi,    utoi,   utoi,   utoi,   utou,   utou,   utou,   skip,   utof,   utof,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL},
    [TY_F32]    = {ftoi,    ftoi,   ftoi,   ftoi,   ftou,   ftou,   ftou,   ftou,   skip,   ftof,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL},
    [TY_F64]    = {ftoi,    ftoi,   ftoi,   ftoi,   ftou,   ftou,   ftou,   ftou,   ftof,   skip,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL},
    [TY_BOOL]   = {NULL,    NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   skip,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL},
    [TY_CHAR]   = {NULL,    NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   skip,   NULL,   NULL,   NULL,   NULL,   NULL},
    [TY_VOID]   = {NULL,    NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   skip,   NULL,   NULL,   NULL,   NULL},
    [TY_PTR]    = {NULL,    NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL},
    [TY_ARR]    = {NULL,    NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL},  
    [TY_ENUM]   = {NULL,    NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL},  
    [TY_STRUCT] = {NULL,    NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL},
};

static LLVMValueRef skip(LLVMCodegenData_T* cg, ASTNode_T* cast)
{
    return llvm_gen_expr(cg, cast->left);
}

static LLVMValueRef itoi(LLVMCodegenData_T* cg, ASTNode_T* cast) 
{
    return LLVMBuildIntCast(cg->llvm_builder, llvm_gen_expr(cg, cast->left), llvm_gen_type(cg, cast->data_type), "itoi");
}

static LLVMValueRef ftof(LLVMCodegenData_T* cg, ASTNode_T* cast) 
{
    return LLVMBuildFPCast(cg->llvm_builder, llvm_gen_expr(cg, cast->left), llvm_gen_type(cg, cast->data_type), "ftof");
}

static LLVMValueRef ftoi(LLVMCodegenData_T* cg, ASTNode_T* cast)
{
    return LLVMBuildFPToSI(cg->llvm_builder, llvm_gen_expr(cg, cast->left), llvm_gen_type(cg, cast->data_type), "ftoi");
}

static LLVMValueRef ftou(LLVMCodegenData_T* cg, ASTNode_T* cast)
{
    return LLVMBuildFPToUI(cg->llvm_builder, llvm_gen_expr(cg, cast->left), llvm_gen_type(cg, cast->data_type), "ftou");
}

static LLVMValueRef itof(LLVMCodegenData_T* cg, ASTNode_T* cast)
{
    return LLVMBuildSIToFP(cg->llvm_builder, llvm_gen_expr(cg, cast->left), llvm_gen_type(cg, cast->data_type), "itof");
}

static LLVMValueRef utof(LLVMCodegenData_T* cg, ASTNode_T* cast)
{
    return LLVMBuildUIToFP(cg->llvm_builder, llvm_gen_expr(cg, cast->left), llvm_gen_type(cg, cast->data_type), "utof");
}

static LLVMValueRef utou(LLVMCodegenData_T* cg, ASTNode_T* cast)
{
    return LLVMBuildIntCast(cg->llvm_builder, llvm_gen_expr(cg, cast->left), llvm_gen_type(cg, cast->data_type), "utou");
}

static LLVMValueRef utoi(LLVMCodegenData_T* cg, ASTNode_T* cast)
{
    return LLVMBuildZExt(cg->llvm_builder, llvm_gen_expr(cg, cast->left), llvm_gen_type(cg, cast->data_type), "utoi");
}

static LLVMValueRef itou(LLVMCodegenData_T* cg, ASTNode_T* cast)
{
    return LLVMBuildIntCast(cg->llvm_builder, llvm_gen_expr(cg, cast->left), llvm_gen_type(cg, cast->data_type), "itou");
}