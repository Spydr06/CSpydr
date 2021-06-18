#include "c_codegen.h"

#include "../../io/log.h"
#include "../../io/io.h"

#include <llvm-c/Target.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define CC_FLAGS "-O3 -Wall -fPIC"

CCodegenData_T* init_c_cg(ASTProg_T* ast)
{
    CCodegenData_T* cg = malloc(sizeof(struct C_CODEGEN_DATA_STRUCT));
    cg->ast = ast;
    cg->print_c = false;
    cg->silent = false;

    cg->code_buffer = open_memstream(&cg->buf, &cg->buf_len);

    return cg;
}

void free_c_cg(CCodegenData_T* cg)
{
    free(cg);
}

static void println(CCodegenData_T* cg, char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vfprintf(cg->code_buffer, fmt, va);
    va_end(va);
    fprintf(cg->code_buffer, "\n");
}

static void write_code(CCodegenData_T* cg, const char* target_bin);

void c_gen_code(CCodegenData_T* cg, const char* target)
{
    if(!cg->silent)
        LOG_OK(COLOR_BOLD_BLUE "  Generating" COLOR_RESET " C Code\n");

    println(cg, "Hello World");
    write_code(cg, target);

    if(cg->print_c)
        LOG_INFO_F("%s\n", cg->buf);
}

static void write_code(CCodegenData_T* cg, const char* target_bin)
{
    const char* file_tmp = "%s.c";
    char* c_file_path = calloc(strlen(file_tmp) + strlen(target_bin) + 1, sizeof(char));
    sprintf(c_file_path, file_tmp, target_bin);

    fclose(cg->code_buffer);

    FILE* out = open_file(c_file_path);
    fwrite(cg->buf, cg->buf_len, 1, out);
    fclose(out);

    free(c_file_path);
}

void run_c_code(CCodegenData_T* cg)
{
    
}