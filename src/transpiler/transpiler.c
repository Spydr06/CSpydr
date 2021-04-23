#include "transpiler.h"
#include "../log.h"

#include <string.h>

#define WRITE(str, target) target = realloc(target, (strlen(target) + strlen(str) + 1) * sizeof(char)); strcat(target, str);
#define WRITE_INCLUDE(str) WRITE(str, transpiler->includeSection)
#define WRITE_DEFINE(str) WRITE(str, transpiler->defineSection)
#define WRITE_CODE(str) WRITE(str, transpiler->codeSection)

transpiler_T* initTranspiler(char* stdPath)
{
    transpiler_T* transpiler = calloc(1, sizeof(struct TRANSPILER_STRUCT));
    transpiler->includeSection = calloc(1, sizeof(char));
    transpiler->defineSection = calloc(1, sizeof(char));
    transpiler->codeSection = calloc(1, sizeof(char));
    transpiler->stdPath = stdPath;

    return transpiler;
}


char* emitCode(transpiler_T* transpiler)
{
    WRITE_INCLUDE(transpiler->defineSection);
    WRITE_INCLUDE(transpiler->codeSection);

    free(transpiler->codeSection);
    free(transpiler->defineSection);
    
    return transpiler->includeSection;
}

static void transpileGlobal(ASTGlobal_T* ast, transpiler_T* transpiler);
static void transpileFunction(ASTFunction_T* ast, transpiler_T* transpiler);
static char* transpileDataType(ASTDataType_T* ast, transpiler_T* transpiler);

void transpileAST(ASTRoot_T* ast, transpiler_T* transpiler)
{
    WRITE_INCLUDE("#include <");
    WRITE_INCLUDE(transpiler->stdPath);
    WRITE_INCLUDE(">\n");

    for(int i = 0; i < ast->globals->size; i++)
    {
        transpileGlobal(ast->globals->items[i], transpiler);
    }

    for(int i = 0; i < ast->functions->size; i++)
    {
        transpileFunction(ast->functions->items[i], transpiler);
    }
}

static char* transpileExpr(ASTExpr_T* ast, transpiler_T* transpiler)
{
    char* expr = calloc(1, sizeof(char));
    return expr;
}

static void transpileLocal(ASTLocal_T* ast, transpiler_T* transpiler)
{
    WRITE_CODE(transpileDataType(ast->dataType, transpiler));
    WRITE_CODE(ast->name);

    if(ast->value != NULL)
    {
        WRITE_CODE("=");
        char* value = transpileExpr(ast->value, transpiler);
        WRITE_CODE(value);
        free(value);
    }

    WRITE_CODE(";\n");
}

static void transpileGlobal(ASTGlobal_T* ast, transpiler_T* transpiler)
{
    WRITE_DEFINE(transpileDataType(ast->dataType, transpiler));
    WRITE_DEFINE(ast->name);

    if(ast->value != NULL)
    {
        WRITE_DEFINE("=");
        char* value = transpileExpr(ast->value, transpiler);
        WRITE_DEFINE(value);
        free(value);
    }

    WRITE_DEFINE(";\n");
}

static void transpileCompoundInstruction(ASTCompoundInstruction_T* ast, transpiler_T* transpiler)
{
    switch(ast->type)
    {
        case AST_CI_LOCALDEF:
            transpileLocal(ast->ptr, transpiler);
            break;
        default:
            break;
    }
}

static void transpileCompound(ASTCompound_T* ast, transpiler_T* transpiler)
{
    WRITE_CODE("{\n");
    
    for(int i = 0; i < ast->body->size; i++)
    {
        transpileCompoundInstruction(ast->body->items[i], transpiler);
    }

    WRITE_CODE("}\n");
}

static void transpileFunction(ASTFunction_T* ast, transpiler_T* transpiler)
{
    char* declaration = calloc(1, sizeof(char));

    if(strcmp(ast->name, "main") == 0)
    {
        const char* mainFunc = "int main(int argc, char** argv)";
        declaration = realloc(declaration, (strlen(mainFunc) + 1) * sizeof(char));
        strcpy(declaration, mainFunc);
    }
    else 
    {
        const char* declarationTemplate = "%s%s(%s)";
        char* returnType = transpileDataType(ast->returnType, transpiler);

        char* arguments = calloc(1, sizeof(char));
        for(int i = 0; i < ast->args->size; i++)
        {
            const char* argTemplate = "%s,%s%s";
            char* dataType = transpileDataType(((ASTArgument_T*) ast->args->items[i])->dataType, transpiler);
            arguments = realloc(arguments, (strlen(arguments) + strlen(dataType) + strlen(argTemplate) + strlen(((ASTArgument_T*)ast->args->items[i])->name) + 1) * sizeof(char));       
            sprintf(arguments, argTemplate, arguments, dataType, ((ASTArgument_T*)ast->args->items[i])->name);
        }   
        if(strlen(arguments) > 1)
        {
            arguments += 1;
        }

        declaration = realloc(declaration, (strlen(declarationTemplate) + strlen(ast->name) + strlen(returnType) + strlen(arguments) + 1) * sizeof(char));
        sprintf(declaration, declarationTemplate, returnType, ast->name, arguments);
    }

    WRITE_DEFINE(declaration);
    WRITE_DEFINE(";\n");

    WRITE_CODE(declaration);

    //function body
    transpileCompound(ast->body, transpiler);

    free(declaration);
}

static char* transpileDataType(ASTDataType_T* ast, transpiler_T* transpiler)
{
    switch(ast->basicType)
    {
        case AST_I32:
            return "int32_t ";
        case AST_I64:
            return "int64_t ";
        case AST_U32:
            return "uint32_t ";
        case AST_U64:
            return "uint64_t ";
        case AST_F32:
            return "float ";
        case AST_F64:
            return "double ";
        case AST_BOOL:
            return "bool ";
        case AST_VOID:
            return "void ";
        case AST_CHAR:
            return "char ";
        case AST_STR:
            return "std::string ";
        case AST_ARRAY: {
            char* innerType = transpileDataType(ast->innerType, transpiler);
            char* type = calloc(strlen(innerType) + 2, sizeof(char));
            strcpy(type, innerType); 
            strcat(type, "*");
            return type;
        }

        case AST_UNDEF:
        default:
            LOG_ERROR_F("undefined data type \"%d\"\n", ast->basicType);
            exit(1);
    }
}