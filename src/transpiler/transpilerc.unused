#include "transpiler.h"

#include "../log.h"

#include <llvm-c/Core.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const char* inbuildFunctions[] = {
    "print", "printf(\"%%s\\n\", %s)",
};

const unsigned int inbuildFunctionsCount = 1;

transpiler_T* initTranspiler()
{
    transpiler_T* transpiler = calloc(1, sizeof(struct TRANSPILER_STRUCT));
    transpiler->codeSection = calloc(1, sizeof(char));
    transpiler->defineSection = calloc(1, sizeof(char));
    transpiler->includeSection = calloc(1, sizeof(char));
    return transpiler;
}

void writeToInclude(transpiler_T* transpiler ,char* str)
{
    transpiler->includeSection = realloc(transpiler->includeSection, (strlen(transpiler->includeSection) + strlen(str) + 1) * sizeof(char));
    strcat(transpiler->includeSection, str);
}

void writeToDefine(transpiler_T* transpiler, char* str)
{
    transpiler->defineSection = realloc(transpiler->defineSection, (strlen(transpiler->defineSection) + strlen(str) + 1) * sizeof(char));
    strcat(transpiler->defineSection, str);
}

void writeToCode(transpiler_T* transpiler, char* str)
{
    transpiler->codeSection = realloc(transpiler->codeSection, (strlen(transpiler->codeSection) + strlen(str) + 1) * sizeof(char));
    strcat(transpiler->codeSection, str);
}

char* transpilerGetCode(transpiler_T* transpiler)
{
    const char* template = "// includes\n%s\n// declarations\n%s\n// code\n%s";
    char* code = calloc(strlen(transpiler->includeSection) + strlen(transpiler->defineSection) + strlen(transpiler->codeSection) + strlen(template) + 1, sizeof(char));
    sprintf(code, template, transpiler->includeSection, transpiler->defineSection, transpiler->codeSection);

    return code;
}

static void transpileFnDef(transpiler_T* transpiler, AST_T* ast);
static void transpileGlobalDef(transpiler_T* transpiler, AST_T* ast);
static void transpileCompound(transpiler_T* transpiler, AST_T* ast);
static void transpileStmt(transpiler_T* transpiler, AST_T* ast);
static char* transpileAssignment(AST_T* ast);

static char* transpileLocalDef(AST_T* ast);
static char* transpileExpr(AST_T* ast);
static char* transpileConstant(AST_T* ast);
static char* transpileCall(AST_T* ast);

static char* isInbuildFunctionCall(AST_T* ast)
{
    char* returnVal = NULL;
    char* name = ast->expr->name;

    for(int i = 0; i < inbuildFunctionsCount * 2; i += 2)
    {
        const char* funcName = inbuildFunctions[i];
        if(strcmp(name, funcName) == 0 && ast->expr->args->size >= 1)
        {
            char* arg = transpileExpr(ast->expr->args->items[0]);
            printf("here\n");
            returnVal = calloc(strlen(inbuildFunctions[i + 1]) + strlen(arg) + 1, sizeof(char));
            sprintf(returnVal, inbuildFunctions[i + 1], arg);
            return returnVal;
        }
    }

    return returnVal;
}

char* transpileToC(transpiler_T* transpiler, AST_T* root)
{
    writeToInclude(transpiler, "#include<stdio.h>\n#include<stdlib.h>\n#include<string.h>\n#include<stdbool.h>\n");

    for(int i = 0; i < root->root->contents->size; i++)
    {
        AST_T* currentAST = root->root->contents->items[i];

        if(currentAST->def->isFunction)
        {
            transpileFnDef(transpiler, currentAST);
        } else 
        {
            transpileGlobalDef(transpiler, currentAST);
        }
    }

    return transpilerGetCode(transpiler);
}

static void transpileFnDef(transpiler_T* transpiler, AST_T* ast)
{
    const char* template = "%s %s(%s)";
    char* returnType = dataTypeToCTypeToString(ast->def->dataType);
    char* name = ast->def->name;

    char* args = calloc(1, sizeof(char));
    for(int i = 0; i < ast->def->args->size; i++)
    {
        char* next = transpileLocalDef(ast->def->args->items[i]);
        
        const char* argTemplate = "%s, %s";
        args = realloc(args, (strlen(args) + strlen(argTemplate) + strlen(next) + 1) * sizeof(char));
        sprintf(args, argTemplate, args, next);
    }
    if(strlen(args) > 2)
    {
        args += 2;
    }

    char* baseFunc = calloc(strlen(template) + strlen(returnType) + strlen(name) + strlen(args) + 1, sizeof(char));
    sprintf(baseFunc, template, returnType, name, args);

    writeToDefine(transpiler, baseFunc);
    writeToDefine(transpiler, ";\n");

    writeToCode(transpiler, baseFunc);
    transpileCompound(transpiler, ast->def->value);
}

static void transpileGlobalDef(transpiler_T* transpiler, AST_T* ast)
{
    char* type = dataTypeToCTypeToString(ast->def->dataType);
    char* str = calloc(strlen(type) + strlen(ast->def->name) + 2, sizeof(char));
    sprintf(str, "%s %s", type, ast->def->name);

    if(ast->def->value != NULL)
    {
        const char* template = "%s=%s";
        char* value = transpileExpr(ast->def->value);
        str = realloc(str, (strlen(str) + strlen(value) + strlen(template) + 1) * sizeof(char));
        sprintf(str, template, str, value);
    }

    writeToDefine(transpiler, str);
    writeToDefine(transpiler, ";\n");
}

static char* transpileLocalDef(AST_T* ast)
{
    char* type = dataTypeToCTypeToString(ast->def->dataType);
    char* str = calloc(strlen(type) + strlen(ast->def->name) + 2, sizeof(char));
    sprintf(str, "%s %s", type, ast->def->name);

    if(ast->def->value != NULL)
    {
        const char* template = "%s=%s";
        char* value = transpileExpr(ast->def->value);
        str = realloc(str, (strlen(str) + strlen(value) + strlen(template) + 1) * sizeof(char));
        sprintf(str, template, str, value);
    }

    return str;
}

static void transpileCompound(transpiler_T* transpiler, AST_T* ast)
{
    writeToCode(transpiler, "{\n");

    for(int i = 0; i < ast->compound->contents->size; i++)
    {
        AST_T* currentAST = ast->compound->contents->items[i];

        switch(currentAST->type)
        {
            case STMT:
                transpileStmt(transpiler, currentAST);
                break;
            case EXPR:
                writeToCode(transpiler, transpileExpr(currentAST));
                writeToCode(transpiler, ";\n");
                break;
            case DEF:
                writeToCode(transpiler, transpileLocalDef(currentAST));
                writeToCode(transpiler, ";\n");
                break;
            default:
                LOG_ERROR_F("Unexpected AST type '%d'\n", currentAST->type);
                break;
        }
    }

    writeToCode(transpiler, "}\n");
}

static void transpileStmt(transpiler_T* transpiler, AST_T* ast)
{
    switch(ast->stmt->type)
    {
        case IF:
            writeToCode(transpiler, "if(");
            //TODO
            writeToCode(transpiler, ")");
            transpileCompound(transpiler, ast->stmt->ifBody);

            if(ast->stmt->elseBody != NULL)
            {
                writeToCode(transpiler, "else");
                transpileCompound(transpiler, ast->stmt->elseBody);
            }
            break;
        case FOR: {
                //const char* template = "for(%s;%s;%s) {";
                //TODO
            } break;
        case WHILE:
            writeToCode(transpiler, "while(");
            //TODO
            writeToCode(transpiler, ")");
            transpileCompound(transpiler, ast->stmt->body);
            break;
        case RETURN:
            writeToCode(transpiler, "return ");
            writeToCode(transpiler, transpileExpr(ast->stmt->value));
            writeToCode(transpiler, ";\n");
            break;
        case EXIT:
            writeToCode(transpiler, "exit(");
            writeToCode(transpiler, transpileExpr(ast->stmt->value));
            writeToCode(transpiler, ");\n");
    }
}

static char* transpileExpr(AST_T* ast)
{
    char* value = calloc(1, sizeof(char));

    switch(ast->expr->type)
    {
        case CONSTANT: {
            char* next = transpileConstant(ast); 
            value = realloc(value, (strlen(value) + strlen(next) + 1) * sizeof(char));
            strcat(value, next);
        } break;

        case CALL: {
            char* next = transpileCall(ast); 
            value = realloc(value, (strlen(value) + strlen(next) + 1) * sizeof(char));
            strcat(value, next);
        } break;

        case ASSIGN: {
            char* next = transpileAssignment(ast); 
            value = realloc(value, (strlen(value) + strlen(next) + 1) * sizeof(char));
            strcat(value, next);
        }

        case NIL: {
            char* next = "NULL"; 
            value = realloc(value, (strlen(value) + strlen(next) + 1) * sizeof(char));
            strcat(value, next);
        }

        default:
            break;
    }

    return value;
}

static char* transpileAssignment(AST_T* ast)
{
    const char* template = "%s=%s";
    char* right = transpileExpr(ast->expr->op.right);
    char* left = transpileExpr(ast->expr->op.left);

    char* value = calloc(strlen(left) + strlen(right) + strlen(template) + 1, sizeof(char));
    sprintf(value, template, left, right);
    return value;
}

static char* transpileCall(AST_T* ast)
{
    char* value = calloc(1, sizeof(char));

    if(ast->expr->isFunctionCall)
    {
        char* inbuildFunc = isInbuildFunctionCall(ast);

        if(inbuildFunc != NULL)
        {
            value = realloc(value ,(strlen(inbuildFunc) + 1) * sizeof(char));
            strcpy(value, inbuildFunc);
        }
        else {
            char* args = calloc(1, sizeof(char));
            for(int i = 0; i < ast->expr->args->size; i++)
            {
                char* next = transpileExpr(ast->expr->args->items[i]);

                const char* argTemplate = "%s, %s";
                args = realloc(args, (strlen(args) + strlen(argTemplate) + strlen(next) + 1) * sizeof(char));
                sprintf(args, argTemplate, args, next);
            }
            if(strlen(args) > 2)
            {
                args += 2;
            }

            const char* template = "%s(%s)";
            value = realloc(value, (strlen(args) + strlen(template) + strlen(ast->expr->name) + 1) * sizeof(char));
            sprintf(value, template, ast->expr->name, args);
        }
    } else 
    {
        value = realloc(value, (strlen(ast->expr->name) + 1) * sizeof(char));
        strcpy(value, ast->expr->name);
    }

    return value;
}

static char* transpileConstant(AST_T* ast)
{
    char* value = calloc(1, sizeof(char));

    switch(ast->expr->dataType->dataType->type)
    {
        case I8:
        case I16:
        case I32:
        case U8:
        case U16: {
            const char* template = "%d";
            value = realloc(value, (strlen(template) + 1) * sizeof(char));
            sprintf(value, template, ast->expr->intValue);
        } break;

        case I64:
        case U32:
        case U64: {
            const char* template = "%ld";
            value = realloc(value, (strlen(template) + 1) * sizeof(char));
            sprintf(value, template, ast->expr->intValue);
        } break;

        case F32:
        case F64: {
            const char* template = "%f";
            value = realloc(value, (strlen(template) + 1) * sizeof(char));
            sprintf(value, template, ast->expr->floatValue);
        } break;

        case BOOL: {
            char* template = ast->expr->boolValue ? "true" : "false";
            value = realloc(value, (strlen(template) + 1) * sizeof(char));
            strcpy(value, template);
        } break;


        case STR: {
            const char* template = "\"%s\"";
            value = realloc(value, (strlen(template) + 1) * sizeof(char));
            sprintf(value, template, ast->expr->strValue);
        } break;
        
        case CHAR: {
            const char* template = "\'%c\'";
            value = realloc(value, (strlen(template) + 1) * sizeof(char));
            sprintf(value, template, ast->expr->charValue);
        } break;

        case VOID:
            break;

        case VEC:
            break;
    }

    return value;
}