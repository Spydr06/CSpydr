#include "c_compiler.h"
#include "AST.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

CCompiler_T* initCompiler()
{
    CCompiler_T* compiler = calloc(1, sizeof(struct C_COMPILER_STRUCT));
    compiler->includeSection = calloc(1, sizeof(char));
    compiler->initSection = calloc(1, sizeof(char));
    compiler->textSection = calloc(1, sizeof(char));

    return compiler;
}

void putStrToIncludeSection(CCompiler_T* compiler, char* str)
{

}

void putStrToInitSection(CCompiler_T* compiler, char* str)
{

}

void putStrToTextSection(CCompiler_T* compiler, char* str)
{

}

char* compileRoot(CCompiler_T* compiler, AST_T* ast)
{
    const char* startText = "#include <stdio.h>\n"
                            "#include <stdlib.h>\n";

    compiler->includeSection = (char*) realloc(compiler->includeSection, (strlen(startText) + 1) * sizeof(char));
    strcat(compiler->includeSection, startText);

    for(int i = 0; i < ast->children->size; i++)
    {
        AST_T* child = ast->children->items[i];
        char* next = compile(child);

        compiler->textSection = realloc(compiler->textSection, (strlen(compiler->textSection) + strlen(next) + 1) * sizeof(char));
        strcat(compiler->textSection, next);
    }

    char* generatedCode = calloc(strlen(compiler->includeSection) + strlen(compiler->initSection) + strlen(compiler->textSection) + 1, sizeof(char));
    sprintf(generatedCode, "%s%s%s", compiler->includeSection, compiler->initSection, compiler->textSection); 

    return generatedCode;
}

char* compile(AST_T* ast)
{
    char* value = calloc(1, sizeof(char));
    char* next = calloc(1, sizeof(char));

    switch(ast->type)
    {
        case AST_COMPOUND:
            next = compileCompound(ast);
            break;
        case AST_VARIABLE:
            next = compileVariable(ast);
            break;
        case AST_ASSIGNMENT: 
            break;
        case AST_STATEMENT:
            next = compileStmt(ast);
            break;
        case AST_FUNCTION:
            next = compileFunction(ast);
            break;
        case AST_CALL:
            next = compileCall(ast);
            break;
        case AST_INT:
            next = compileInt(ast);
            break;
        case AST_STRING:
            next = compileString(ast);
            break;
        default:
            printf("[C Compiler]: No frontend for AST of type '%d'\n", ast->type);
            exit(1);
            break;
    }

    value = realloc(value, (strlen(next) + 1) * sizeof(char));
    strcat(value, next);

    return value;
}

char* compileCall(AST_T* ast)
{
    char* value = calloc(strlen(ast->name) + 1, sizeof(char));
    strcpy(value, ast->name);

    char* list = calloc(1, sizeof(char));
    for(int i = 0; i < ast->value->children->size; i++)
    {
        AST_T* arg = (AST_T*) ast->value->children->items[i];
        char* argv = compile(arg);

        list = realloc(list, (strlen(list) + strlen(argv) + 2) * sizeof(char));
        sprintf(list, "%s,%s", list, argv);
    }
    if(strlen(list) > 2) {
        list += 1;
    }

    value = realloc(value, (strlen(value) + strlen(list) + 5) * sizeof(char));
    sprintf(value, "%s(%s);\n", value, list);
    return value;
}

char* compileStmt(AST_T* ast)
{
    char* value = calloc(strlen(ast->name) + 1, sizeof(char));
    strcpy(value, ast->name);

    if(strcmp(value, "return") == 0) {
        char* next = compile(ast->value);
        value = realloc(value, (strlen(value) + strlen(next) + 4) * sizeof(char));
        sprintf(value, "%s %s;\n", value, next);
    }

    return value;
}

char* compileInt(AST_T* ast) 
{
    char* value = calloc(13, sizeof(char));
    sprintf(value, "%d", ast->intValue);
    return value;
}

char* compileString(AST_T* ast) 
{
    char* value = calloc(strlen(ast->strValue) + 3, sizeof(char));
    sprintf(value, "\"%s\"", ast->strValue);
    return value;
}

char* compileVariable(AST_T* ast)
{
    char* value = resolveType(ast->dataType, ast->name);

    if(ast->value == NULL) {
        value = (char*) realloc(value, (strlen(value) + strlen(";\n") + 1) * sizeof(char));
        strcat(value, ";\n");
        return value;
    }

    char* varValue = compile(ast->value);
    value = realloc(value, (strlen(value) + strlen(varValue) + 4) * sizeof(char));
    sprintf(value, "%s=%s;\n", value, varValue);
    return value;
}

char* compileFunction(AST_T* ast)
{
    char* type = resolveType(ast->dataType, ast->name);
    char* list = calloc(1, sizeof(char));

    for(int i = 0; i < ast->children->size; i++)
    {
        AST_T* arg = (AST_T*) ast->children->items[i];
        char* argType = resolveType(arg->dataType, arg->name);

        list = realloc(list, (strlen(list) + strlen(argType) + 2) * sizeof(char));
        sprintf(list, "%s,%s", list, argType);
    }
    if(strlen(list) > 2) {
        list += 1;
    }

    char* contents = compile(ast->value);

    const char* template = "%s(%s) {\n"
                           "%s"
                           "}\n";

    char* value = calloc(strlen(template) + strlen(type) + strlen(list) + strlen(contents) + 1, sizeof(char));
    sprintf(value, template, type, list, contents);

    return value;
}

char* compileCompound(AST_T* ast) 
{
    char* value = calloc(1, sizeof(char));
    
    for(int i = 0; i < ast->children->size; i++)
    {
        char* next = compile(ast->children->items[i]);
        value = realloc(value, (strlen(value) + strlen(next) + 1) * sizeof(char));
        strcat(value, next);
    }
    return value;
}

char* resolveSimpleType(char* type)
{
    switch(type[0])
    {
        case 'i':
            return "int";
            break;
        case 's':
            return "char*";
            break;
        default:
            return "";
    }
}

char* resolveType(char* dataType, char* varName)
{
    bool isArray = false;
    char* type = calloc(1, sizeof(char));
    switch(dataType[0])
    {
        case 'v': {
            isArray = true;
            if(strlen(dataType) <= 3) {
                printf("[Syntax Error]: type '%s' not specified.", dataType);
                exit(1);
            }
            dataType += 3;
        }
        default:
            type = resolveSimpleType(dataType);
    }

    char* name = varName;

    char* value = (char*) calloc(strlen(type) + strlen(name) + 2, sizeof(char));
    sprintf(value, "%s %s", type, name);

    if(isArray)
    {
        value = (char*) realloc(value, (strlen(value) + 3) * sizeof(char));
        strcat(value, "[]");
    }

    return value;
}