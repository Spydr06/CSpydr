#ifndef CSPYDR_ASM_CODEGEN_H
#define CSPYDR_ASM_CODEGEN_H

#include "../../ast/ast.h"
#include <stdio.h>

typedef struct ASM_CODEGEN_DATA_STRUCT
{
    ASTProg_T* ast;
    bool silent;
    bool print;

    char* buf;
    size_t buf_len;
    FILE* code_buffer;

    ASTObj_T* current_fn;
    char* current_fn_name;
    size_t depth;
} ASMCodegenData_T;

void init_asm_cg(ASMCodegenData_T* cg, ASTProg_T* ast);
void asm_gen_code(ASMCodegenData_T* cg, const char* target);

#endif