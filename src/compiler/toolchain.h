#ifndef CSPYDR_TOOLCHAIN_H
#define CSPYDR_TOOLCHAIN_H

#include "ast/ast.h"
#include "config.h"
#include "context.h"

typedef struct PASS_STRUCT {
    const char* desc;
    i32 (*func)(Context_T* context, ASTProg_T* ast);
} Pass_T;

void compile(Context_T* context, char* input_file, char* output_file);

#endif
