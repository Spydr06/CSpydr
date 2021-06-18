#include "c_codegen.h"

#include "../../io/log.h"
#include "../../io/io.h"

#include <llvm-c/Target.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "../../platform/platform_bindings.h"

#define CC "gcc"
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
static void run_compiler(CCodegenData_T* cg,  const char* target_bin);

void c_gen_code(CCodegenData_T* cg, const char* target)
{
    if(!cg->silent)
        LOG_OK(COLOR_BOLD_BLUE "  Generating" COLOR_RESET " C code using " COLOR_BOLD_WHITE CC COLOR_RESET "\n");

    println(cg, "int main() {return 0;}");
    write_code(cg, target);

    if(cg->print_c)
        LOG_INFO_F("%s", cg->buf);
    
    run_compiler(cg, target);
}

static void run_compiler(CCodegenData_T* cg, const char* target_bin)
{

    static char* compiler_cmd_tmp = CC " " CC_FLAGS " %s.c -o %s";
    char* compiler_cmd = calloc(strlen(compiler_cmd_tmp) + strlen(target_bin) * 2 + 1, sizeof(char));
    sprintf(compiler_cmd, compiler_cmd_tmp, target_bin, target_bin);

    char* feedback = sh(compiler_cmd);
    if(!cg->silent)
        LOG_INFO_F("%s", feedback);

    free(compiler_cmd);
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

void run_c_code(CCodegenData_T* cg, const char* bin)
{
    if(!cg->silent)
        LOG_OK_F(COLOR_BOLD_BLUE "  Executing " COLOR_RESET " %s\n", bin);
    
    const char* cmd_tmp = "." DIRECTORY_DELIMS "%s";
    char* cmd = calloc(strlen(cmd_tmp) + strlen(bin) + 1, sizeof(char));
    sprintf(cmd, cmd_tmp, bin);

    char* feedback = sh(cmd);
    if(!cg->silent)
        LOG_OK_F("%s", feedback);
    
#ifdef __linux
    if(!cg->silent)
    {
        int exit_code = atoi(sh("echo $?"));
        LOG_INFO_F("\"%s\" terminated with exit code %d.\n", cg->ast->main_file_path ,exit_code);
    }
#endif

    free(cmd);
}