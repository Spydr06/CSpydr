#ifndef CSPYDR_C_CODEGEN_H
#define CSPYDR_C_CODEGEN_H

#include "ast/ast.h"

extern char* cc;
extern char* cc_flags;

typedef struct C_CODEGEN_DATA_STRUCT
{
    ASTProg_T* ast;

    bool print;
    bool silent;
    u64 unique_id;
    
    HashMap_T* arrays;
    List_T* blocks;

    char* buf;
    size_t buf_len;
    FILE* code_buffer;
} CCodegenData_T;

void init_c_cg(CCodegenData_T* cg, ASTProg_T* ast);
void free_c_cg(CCodegenData_T* cg);

void c_gen_code(CCodegenData_T* cg, const char* target);

#endif