#include "asm_codegen.h"
#include "../../io/log.h"
#include "../../io/io.h"
#include "../../ast/ast_iterator.h"
#include "../../platform/platform_bindings.h"

static void generate_files(ASMCodegenData_T* cg);

void init_asm_cg(ASMCodegenData_T* cg, ASTProg_T* ast)
{
    cg->ast = ast;
    cg->print = false;
    cg->silent = false;

    cg->code_buffer = open_memstream(&cg->buf, &cg->buf_len);
}

static void println(ASMCodegenData_T* cg, char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vfprintf(cg->code_buffer, fmt, va);
    va_end(va);
    fprintf(cg->code_buffer, "\n");
}

static void print(ASMCodegenData_T* cg, char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vfprintf(cg->code_buffer, fmt, va);
}

static void write_code(ASMCodegenData_T* cg, const char* target_bin)
{
    char* homedir = get_home_directory();
    char cache_dir[BUFSIZ] = {'\0'};
    sprintf(cache_dir, "%s" DIRECTORY_DELIMS CACHE_DIR DIRECTORY_DELIMS, homedir);

    if(make_dir(cache_dir))
    {
        LOG_ERROR("error creating cache directory `" DIRECTORY_DELIMS CACHE_DIR DIRECTORY_DELIMS "`.\n");
        exit(1);
    }

    char c_file_path[BUFSIZ * 2] = {'\0'};
    sprintf(c_file_path, "%s" DIRECTORY_DELIMS "%s.a", cache_dir, target_bin);

    fclose(cg->code_buffer);

    FILE* out = open_file(c_file_path);
    fwrite(cg->buf, cg->buf_len, 1, out);
    fclose(out);
}

void asm_gen_code(ASMCodegenData_T* cg, const char* target)
{
    if(!cg->silent)
    {
        LOG_OK(COLOR_BOLD_BLUE "  Generating" COLOR_BOLD_WHITE " Assembly" COLOR_RESET " code\n");
    }

    write_code(cg, target);

    if(cg->print)
        LOG_INFO_F("%s", cg->buf);

    // run the assembler
    char asm_source_file[BUFSIZ] = {'\0'};
    sprintf(asm_source_file, "%s" DIRECTORY_DELIMS CACHE_DIR DIRECTORY_DELIMS "%s.c", get_home_directory(), target);
}