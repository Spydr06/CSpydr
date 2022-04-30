#ifndef CSPYDR_TOOLCHAIN_H
#define CSPYDR_TOOLCHAIN_H

typedef enum COMPILE_TYPE_ENUM
{
    CT_TRANSPILE,
    CT_ASM,
    CT_TO_JSON,
} CompileType_T;

typedef enum ACTION_ENUM
{
    AC_NULL = -1,
    AC_BUILD,
    AC_RUN,
    AC_DEBUG
} Action_T;

void compile(char* input_file, char* output_file, Action_T action);

#endif