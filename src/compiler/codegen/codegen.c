#include "codegen.h"
#include "config.h"
#include "io/log.h"
#include "ir/ir.h"
#include "backend.h"

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

i32 codegen_pass(Context_T* context, IR_T* ir)
{
    CodegenData_T c;
    init_codegen_data(&c, context, ir);
    c.b->begin_file(&c);    

    
    c.b->finish_file(&c);

    free_codegen_data(&c);
    printf("Codegen not implemented.\n");
    exit(1);
    return 0;
}

