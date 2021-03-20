#include "ASTWalker.h"
#include "compiler.h"
#include "../log.h"

#include "llvm.h"
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Types.h>

static void walkFnDefinition(LLVMModuleRef mod, AST_T* ast);
static void walkGlobalVarDefinition(LLVMModuleRef mod, AST_T* ast);

static void walkCompound(LLVMModuleRef mod, LLVMValueRef function, AST_T* ast, char* entryName);
static void walkStatement(LLVMModuleRef mod, LLVMValueRef function, LLVMBuilderRef parentBuilder, AST_T* ast);

static LLVMValueRef walkExpression(LLVMModuleRef mod, AST_T* ast);
static LLVMValueRef walkConstant(LLVMModuleRef mod, AST_T* ast);

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
                    walkGlobalVarDefinition(mod, currentAST);
                }
                break;

            default:
                LOG_ERROR(COLOR_BOLD_RED "Illegal AST type '%d' found in root.\n", currentAST->type);
                exit(1);
        }
        
    }

}

static void walkGlobalVarDefinition(LLVMModuleRef mod, AST_T* ast)
{
    LLVMValueRef global = LLVMAddGlobal(mod, dataTypeToLLVMTypeRef(ast->def->dataType)->type, ast->def->name);
    if(ast->def->value != NULL)
    {
        //TODO
       LLVMValueRef expression = walkExpression(mod, ast->def->value);
       LLVMSetInitializer(global, expression);
    }
}

static void walkLocalVarDefinition(LLVMModuleRef mod, LLVMBuilderRef builder, AST_T* ast)
{
    valueType_T* type = dataTypeToLLVMTypeRef(ast->def->dataType);
    LLVMBuildAlloca(builder, type->type, ast->def->name);
}

static void walkFnDefinition(LLVMModuleRef mod, AST_T* ast)
{
    LLVMTypeRef* params = calloc(1, sizeof(LLVMTypeRef));
    int size = 0;
    for(int i = 0; i < ast->def->args->size; i++)
    {
        AST_T* arg = ast->def->args->items[i];
        size++;
        params = realloc(params, sizeof(LLVMTypeRef) * size);
        params[size - 1] = dataTypeToLLVMTypeRef(arg->def->dataType)->type;
    }

    LLVMTypeRef returnType = LLVMFunctionType(dataTypeToLLVMTypeRef(ast->def->dataType)->type, params, size, 0);
    LLVMValueRef function = LLVMAddFunction(mod, ast->def->name, returnType);

    walkCompound(mod, function, ast->def->value, "entry");
}

static void walkCompound(LLVMModuleRef mod, LLVMValueRef function, AST_T* ast, char* entryName)
{
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(function, entryName);
    LLVMBuilderRef builder = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(builder, entry);

    for(int i = 0; i < ast->compound->contents->size; i++)
    {
        AST_T* currentAST = ast->compound->contents->items[i];

        switch (currentAST->type) {
            case DEF:
                walkLocalVarDefinition(mod, builder, currentAST);
                break;
            case STMT:
                walkStatement(mod, function, builder, currentAST);
                break;
            case EXPR:
                break;
            default:
                LOG_ERROR("Invalid AST type '%d' found in compound.\n", currentAST->type);
                exit(1);
        }
    }

    LLVMDisposeBuilder(builder);
}

static void walkStatement(LLVMModuleRef mod, LLVMValueRef function, LLVMBuilderRef parentBuilder, AST_T* ast)
{
    switch(ast->stmt->type)
    {
        case RETURN: {
            LLVMValueRef tmp = LLVMGetParam(function, 0);
            LLVMBuildRet(parentBuilder, tmp);
        } break;
        case EXIT:
            //LLVMBuildCall(parentBuilder, function, , unsigned int NumArgs, const char *Name)
            break;
        case FOR:
            break;
        case WHILE:
            break;
        case IF:
            break;
        default:
            LOG_ERROR("Undefined Statment type '%d'.\n", ast->stmt->type);
            exit(1);
    }
}

static LLVMValueRef walkAssignment(LLVMModuleRef mod, LLVMBuilderRef ref, AST_T* ast)
{
    LLVMValueRef right = walkExpression(mod, ast->expr->op.right);
   // LLVMBuildAlloca(builder, type->type, ast->def->name);
}

static LLVMValueRef walkExpression(LLVMModuleRef mod, AST_T* ast)
{
    switch(ast->expr->type)
    {
        case CONSTANT:
            return walkConstant(mod, ast);
        default:
            LOG_ERROR("expressions with the type '%d' are currently not supported.\n", ast->expr->type);
            exit(1);
    }
}

static LLVMValueRef walkConstant(LLVMModuleRef mod, AST_T* ast)
{
    valueType_T* type = dataTypeToLLVMTypeRef(ast->expr->dataType);
    LLVMValueRef ref;
    switch(type->kind)
    {
        case INT:
            ref = LLVMConstInt(type->type, ast->expr->intValue, type->isSigned);
            break;
        case BOOLEAN:
            ref = LLVMConstInt(type->type, ast->expr->boolValue, type->isSigned);
            break;
        case FLOAT:
            ref = LLVMConstReal(type->type, ast->expr->floatValue);
            break;
        case STRING:
            //TODO
            break;
        case POINTER:
            //TODO
            break;
        default:
            LOG_ERROR("Undefined constant kind '%d'.\n", type->kind);
            exit(1);
    }

    return ref;
}