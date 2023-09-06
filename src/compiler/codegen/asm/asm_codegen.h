#ifndef CSPYDR_ASM_CODEGEN_H
#define CSPYDR_ASM_CODEGEN_H

#include <stdio.h>

#include "ast/ast.h"
#include "config.h"

typedef struct ASM_CODEGEN_DATA_STRUCT
{
    Context_T* context;
    ASTProg_T* ast;
    bool silent;
    bool print;
    bool embed_file_locations;
    bool link_exec;

    char* buf;
    size_t buf_len;
    FILE* code_buffer;

    ASTObj_T* current_fn;
    char* current_fn_name;
    u64 depth;

    List_T* string_literals;

    u64 max_count;  // current maximum label id
    u64 cur_count;  // current label id
    u64 cur_brk_id; // current statement id, which supports break; statements
    u64 cur_cnt_id; // current statement id, which supports continue; statements
} ASMCodegenData_T;

i32 asm_codegen_pass(Context_T* context, ASTProg_T* ast);

void init_asm_cg(ASMCodegenData_T* cg, Context_T* context, ASTProg_T* ast);
void free_asm_cg(ASMCodegenData_T* cg);
void asm_gen_code(ASMCodegenData_T* cg, const char* target);

char* asm_gen_identifier(ASTIdentifier_T* id);

#ifdef __GNUC__
__attribute((format(printf, 2, 3)))
#endif
void asm_print(ASMCodegenData_T* cg, char* fmt, ...);

#ifdef __GNUC__
__attribute((format(printf, 2, 3)))
#endif
void asm_println(ASMCodegenData_T* cg, char* fmt, ...);

#endif