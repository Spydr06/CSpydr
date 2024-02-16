#include "codegen.h"
#include "config.h"
#include "error/error.h"
#include "io/io.h"
#include "io/log.h"
#include "ir/ir.h"
#include "backend.h"
#include "timer/timer.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static void init_codegen_data(CodegenData_T* c, Context_T* context, IR_T* ir)
{
    memset(c, 0, sizeof(struct CODEGEN_DATA_STRUCT));

    c->context = context;
    c->ir = ir;

    c->generate_debug_info = context->flags.embed_debug_info;
    if(c->generate_debug_info && !context->backend->supports_debug_info)
        LOG_WARN_F(COLOR_BOLD_YELLOW "[Warning]" COLOR_RESET COLOR_YELLOW "`%s`: Debugging information cannot be generated.\n" COLOR_RESET, context->backend->name);

    c->output.code_buffer = open_memstream(&c->output.buf, &c->output.buf_len);

    c->b = context->backend->callbacks;
}

static void free_codegen_data(CodegenData_T* c)
{
    free(c->output.buf);
}

#ifdef __GNUC__
__attribute__((format(printf, 2, 3)))
#endif
void codegen_printf(CodegenData_T* c, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(c->output.code_buffer, fmt, ap);
    va_end(ap);
}

#ifdef __GNUC__
__attribute__((format(printf, 2, 3)))
#endif
void codegen_println(CodegenData_T* c, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(c->output.code_buffer, fmt, ap);
    va_end(ap);
    fputc('\n', c->output.code_buffer);
}

#define CODEGEN_WRITE_FUNC(type)                                \
    void codegen_write_##type(CodegenData_T* c, type val)       \
    {                                                           \
        fwrite(&val, 1, sizeof(type), c->output.code_buffer);   \
    }

CODEGEN_WRITE_FUNC(u8);
CODEGEN_WRITE_FUNC(u16);
CODEGEN_WRITE_FUNC(u32);
CODEGEN_WRITE_FUNC(u64);

static void dump_generated_code(CodegenData_T* c)
{
    switch(c->context->backend->output_format)
    {
    case OUTPUT_FORMAT_TEXT:
        fprintf(OUTPUT_STREAM, "%s", c->output.buf);
        fflush(stdout);
        break;
    case OUTPUT_FORMAT_RAW:
        // TODO: hexdump
        break;
    default:
        unreachable();
    }
}

i32 codegen_pass(Context_T* context, IR_T* ir, const char* target, const char** object_path)
{
    timer_start(context, "code generation");

    CodegenData_T c;
    init_codegen_data(&c, context, ir);
    c.b->begin_file(&c);    
    

    c.b->finish_file(&c); 
    fclose(c.output.code_buffer);

    if(context->flags.verbose)
    {
        LOG_INFO(COLOR_BOLD_MAGENTA ">> Code generation:\n" COLOR_RESET);
        dump_generated_code(&c);
    }

    char* output_filepath = malloc(BUFSIZ * sizeof(char));
    *output_filepath = '\0';
    get_cached_file_path(output_filepath, target, context->backend->fileext);
    
    {
        FILE* output_file = open_file(output_filepath);
        fwrite(c.output.buf, c.output.buf_len, sizeof(u8), output_file);
        fclose(output_file);
    }

    timer_stop(context);

    char* object_filepath = NULL;
    if(!context->flags.do_assembling)
        goto finish;
    
    timer_start(context, "compiling");

    object_filepath = output_filepath;
    if(strcmp(context->backend->fileext, ".o") != 0)
    {
        if(!context->flags.silent)
            LOG_INFO_F(COLOR_BOLD_BLUE "  Compiling " COLOR_RESET " %s\n", context->backend->name);

        object_filepath = malloc(BUFSIZ * sizeof(char));
        get_cached_file_path(object_filepath, target, ".o");

        c.b->compile(&c, output_filepath, object_filepath);
    }

    timer_stop(context);

finish:
    if(object_filepath != output_filepath)
        free(output_filepath);

    *object_path = object_filepath;
    
    free_codegen_data(&c);
    return 0;
}

