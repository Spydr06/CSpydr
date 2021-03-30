#include "compiler.h"
#include "ASTWalker.h"
#include "../log.h"
#include "llvm.h"

LLVMCompiler_T initCompiler()
{
    LLVMCompiler_T compiler;
    return compiler;
}

void compileProgram(AST_T* root, char* outputFile, char* srcFile)
{
    LLVMCompiler_T compiler;
    compiler.root = LLVMModuleCreateWithName(srcFile);

    //LLVMModuleRef mod = LLVMModuleCreateWithName("test");
    
    walkRoot(compiler.root, root);
    
    printf("here\n");

    char* error = NULL;
    LLVMVerifyModule(compiler.root, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error);

    if(LLVMWriteBitcodeToFile(compiler.root, outputFile) != 0)
    {
        
        LOG_ERROR("Error writing Bitcode to File '%s'.\n", outputFile);
        exit(1);
        
    }
}

valueType_T* initValueType(LLVMTypeRef type, bool isSigned, int kind)
{
    valueType_T* value = calloc(1, sizeof(struct LLVM__VALUE_TYPE));
    value->type = type;
    value->isSigned = isSigned;
    value->kind = kind;
    return value;
}

valueType_T* dataTypeToLLVMTypeRef(AST_T* ast)
{
    switch(ast->dataType->type)
    {
        case I8:
            return initValueType(LLVMInt8Type(), true, INT);
        case I16:
            return initValueType(LLVMInt16Type(), true, INT);
        case I32:
            return initValueType(LLVMInt32Type(), true, INT);
        case I64:
            return initValueType(LLVMInt64Type(), true, INT);
        case F32:
            return initValueType(LLVMFloatType(), true, FLOAT);
        case F64:
            return initValueType(LLVMDoubleType(), true, FLOAT);
        case BOOL:
            return initValueType(LLVMInt8Type(), false, BOOLEAN);
        case CHAR:
            return initValueType(LLVMInt8Type(), true, INT);
        case STR:
            return initValueType(LLVMArrayType(LLVMInt8Type(), 0), true, STRING);
        case VEC:
            return initValueType(LLVMArrayType(dataTypeToLLVMTypeRef(ast->dataType->subtype)->type, 0), true, POINTER);
        case VOID:
            return initValueType(LLVMVoidType(), true, POINTER);
        default:
            LOG_ERROR(COLOR_BOLD_WHITE "The type '%d' is currently not supported.\n", ast->dataType->type);
            exit(1);
    }
}