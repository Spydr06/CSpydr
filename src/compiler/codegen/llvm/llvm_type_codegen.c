#include "llvm_codegen.h"

LLVMTypeRef llvm_gen_type(LLVMCodegenData_T* cg, ASTType_T* ty)
{
    switch(ty->kind)
    {
        case TY_I8: case TY_I16: case TY_I32: case TY_I64:
        case TY_U8: case TY_U16: case TY_U32: case TY_U64:
            return LLVMIntType(ty->size << 3);
        case TY_F32:
            return LLVMFloatType();
        case TY_F64:
            return LLVMDoubleType();
            return NULL;
        case TY_BOOL:
            return LLVMInt1Type();
        case TY_CHAR:
            return LLVMInt8Type();
        case TY_VOID:
            return LLVMVoidType();
        case TY_PTR:
            return LLVMPointerType(llvm_gen_type(cg, ty->base), ty->base->size);
        case TY_ARR:
            return LLVMArrayType(llvm_gen_type(cg, ty->base), 0);
        case TY_ENUM:
            //TODO:
            return NULL;
        case TY_STRUCT:
            return NULL;
        case TY_UNDEF:
        case TY_TUPLE:
        case TY_LAMBDA:
            //TODO:
            return NULL;
    }

    // satisfy -Wall
    return NULL;
}