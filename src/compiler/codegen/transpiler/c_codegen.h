#ifndef CSPYDR_C_CODEGEN_H
#define CSPYDR_C_CODEGEN_H

#include "../../ast/ast.h"

#include <stdio.h>

typedef struct C_CODEGEN_DATA_STRUCT
{
    ASTProg_T* ast;

    bool print_c;
    bool silent;

    char* c_file_path;

    char* buf;
    size_t buf_len;
    FILE* code_buffer;
} CCodegenData_T;

CCodegenData_T* init_c_cg(ASTProg_T* ast);
void            free_c_cg(CCodegenData_T* cg);

void c_gen_code(CCodegenData_T* cg, const char* target);
void run_c_code(CCodegenData_T* cg, const char* bin);

#endif