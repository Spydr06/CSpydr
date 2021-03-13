#ifndef CSPYDR_C_COMPILER_H
#define CSPYDR_C_COMPILER_H

#include "parser.h"

typedef struct C_COMPILER_STRUCT
{
    char* includeSection;
    char* initSection;
    char* textSection;
} CCompiler_T;

CCompiler_T* initCompiler();
void putStrToIncludeSection(CCompiler_T* compiler, char* str);
void putStrToInitSection(CCompiler_T* compiler, char* str);
void putStrToTextSection(CCompiler_T* compiler, char* str);

char* compileRoot(CCompiler_T* compiler, AST_T* ast);
char* compile(AST_T* ast);
char* compileFunction(AST_T* ast);
char* compileVariable(AST_T* ast);
char* compileStmt(AST_T* ast);
char* compileCompound(AST_T* ast);
char* compileInt(AST_T* ast);
char* compileString(AST_T* ast);
char* compileCall(AST_T* ast);

char* resolveType(char* type, char* varName);

#endif