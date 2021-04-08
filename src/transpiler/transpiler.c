#include "transpiler.h"
#include "../log.h"

#include <string.h>

#define WRITE(str, target) target = realloc(target, (strlen(target) + strlen(str) + 1) * sizeof(char)); strcat(target, str);
#define WRITE_INCLUDE(str) WRITE(str, transpiler->includeSection)
#define WRITE_DEFINE(str) WRITE(str, transpiler->defineSection)
#define WRITE_CODE(str) WRITE(str, transpiler->codeSection)

transpiler_T* initTranspiler()
{
    transpiler_T* transpiler = calloc(1, sizeof(struct TRANSPILER_STRUCT));
    transpiler->includeSection = calloc(1, sizeof(char));
    transpiler->defineSection = calloc(1, sizeof(char));
    transpiler->codeSection = calloc(1, sizeof(char));

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
    WRITE_INCLUDE("#include <stdio.h>\n");
    WRITE_INCLUDE("#include <stdlib.h>\n");
    WRITE_INCLUDE("#include <stdbool.h>\n");

    for(int i = 0; i < ast->globals->size; i++)
    {
        transpileGlobal(ast->globals->items[i], transpiler);
    }

    for(int i = 0; i < ast->functions->size; i++)
    {
        transpileFunction(ast->functions->items[i], transpiler);
    }
}

static void transpileFunction(ASTFunction_T* ast, transpiler_T* transpiler)
{
    
}

static void transpileGlobal(ASTGlobal_T* ast, transpiler_T* transpiler)
{
    WRITE_DEFINE(transpileDataType(ast->dataType, transpiler));
    WRITE_DEFINE(ast->name);

    if(ast->value != NULL)
    {

    }

    WRITE_DEFINE(";\n");
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
            return "char* ";
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