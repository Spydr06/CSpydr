#ifndef CSPYDR_TOOLCHAIN_H
#define CSPYDR_TOOLCHAIN_H

#include "ast/ast.h"
#include "config.h"
#include "context.h"

typedef enum COMPILE_TYPE_ENUM
{
    CT_TRANSPILE,
    CT_ASM,
#ifdef CSPYDR_USE_LLVM
    CT_LLVM,
#endif
    CT_TO_JSON,
} CompileType_T;

typedef struct PASS_STRUCT {
    const char* desc;
    i32 (*func)(Context_T* context, ASTProg_T* ast);
} Pass_T;

void compile(Context_T* context, char* input_file, char* output_file);

#endif