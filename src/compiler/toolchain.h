#ifndef CSPYDR_TOOLCHAIN_H
#define CSPYDR_TOOLCHAIN_H

typedef enum COMPILE_TYPE_ENUM
{
    CT_LLVM,
    CT_TRANSPILE,
    CT_TO_XML,
} CompileType_T;

typedef enum ACTION_ENUM
{
    AC_BUILD,
    AC_RUN,
    AC_DEBUG,
    AC_REPL,
    AC_UNDEF
} Action_T;

void compile(char* input_file, char* output_file, Action_T action);

#endif