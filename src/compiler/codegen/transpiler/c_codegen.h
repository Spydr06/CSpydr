#ifndef CSPYDR_C_CODEGEN_H
#define CSPYDR_C_CODEGEN_H

#include "../../ast/ast.h"

#include <stdio.h>

extern char* cc;
extern char* cc_flags;

typedef struct C_CODEGEN_DATA_STRUCT
{
    ASTProg_T* ast;
    ASTObj_T* current_fn;
    ASTNode_T* current_lambda;

    bool print_c;
    bool silent;

    char* c_file_path;

    char* buf;
    size_t buf_len;
    FILE* code_buffer;
} CCodegenData_T;

void init_c_cg(CCodegenData_T* cg, ASTProg_T* ast);
void free_c_cg(CCodegenData_T* cg);

void c_gen_code(CCodegenData_T* cg, const char* target);

#endif