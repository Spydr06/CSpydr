#include "ASTWalker.h"
#include "compiler.h"
#include "../log.h"

#include "llvm.h"
#include <llvm-c/Core.h>
#include <llvm-c/Types.h>

static void walkFnDefinition(LLVMModuleRef mod, AST_T* ast);

void walkRoot(LLVMModuleRef mod, AST_T* ast)
{
    for(int i = 0; i < ast->root->contents->size; i++)
    {
        AST_T* currentAST = ast->root->contents->items[i];

        switch(currentAST->type)
        {
            case DEF:
                if(currentAST->def->isFunction)
                {
                    walkFnDefinition(mod, currentAST);
                } else 
                {

                }
                break;

            default:
                LOG_ERROR(COLOR_BOLD_RED "Illegal AST type '%d' found in root.\n", currentAST->type);
                exit(1);
        }
        
    }

}

static void walkFnDefinition(LLVMModuleRef mod, AST_T* ast)
{
    list_T* arguments = initList(sizeof(LLVMTypeRef));
    LLVMTypeRef* params = calloc(1, sizeof(LLVMTypeRef));
    int size = 0;
    for(int i = 0; i < ast->def->args->size; i++)
    {
        AST_T* arg = ast->def->args->items[i];
        size++;
        params = realloc(params, sizeof(LLVMTypeRef) * size);
        params[size - 1] = dataTypeToLLVMTypeRef(arg->def->dataType);
    }

    LLVMTypeRef returnType = LLVMFunctionType(dataTypeToLLVMTypeRef(ast->def->dataType), params, size, 0);
    LLVMValueRef function = LLVMAddFunction(mod, ast->def->name, returnType);

    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(function, "entry");
    LLVMBuilderRef builder = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(builder, entry);

    LLVMValueRef tmp = LLVMGetParam(function, 0);
    LLVMBuildRet(builder, tmp);

    LLVMDisposeBuilder(builder);
}