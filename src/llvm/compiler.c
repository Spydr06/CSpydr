#include "compiler.h"
#include "ASTWalker.h"
#include "../log.h"
#include "llvm.h"

LLVMCompiler_T initCompiler()
{
    LLVMCompiler_T compiler;
    return compiler;
}

void compileProgram(AST_T* root, char* outputFile)
{
    LLVMCompiler_T compiler;
    compiler.root = LLVMModuleCreateWithName("root");

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

LLVMTypeRef dataTypeToLLVMTypeRef(AST_T* ast)
{
    switch(ast->dataType->type)
    {
        case I8:
            return LLVMInt8Type();
        case I16:
            return LLVMInt16Type();
        case I32:
            return LLVMInt32Type();
        case I64:
            return LLVMInt128Type();
        case F32:
            return LLVMFloatType();
        case F64:
            return LLVMDoubleType();
        case BOOL:
            return LLVMInt8Type();
        case CHAR:
            return LLVMInt8Type();
        case STR:
            return LLVMArrayType(LLVMInt8Type(), 0);
        case VEC:
            return LLVMArrayType(dataTypeToLLVMTypeRef(ast->dataType->subtype), 0);
        case VOID:
            return LLVMVoidType();
        default:
            LOG_ERROR(COLOR_BOLD_WHITE "The type '%d' is currently not supported.\n", ast->dataType->type);
            exit(1);
    }
}