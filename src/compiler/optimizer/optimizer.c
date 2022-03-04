#include "optimizer.h"
#include "ast/ast.h"
#include "config.h"
#include "error/error.h"
#include "io/log.h"

#define throw_error(...)              \
    do {                              \
        fprintf(OUTPUT_STREAM, "\n"); \
        throw_error(__VA_ARGS__);     \
    } while(0)

void eliminate_dead_code(ASTProg_T* ast);

void optimize(ASTProg_T *ast)
{
    static struct {
        void (*fn)(ASTProg_T*);
        const char* description;
    } passes[] = {
        {eliminate_dead_code, "eliminate dead code"},
        {NULL, NULL}
    };

    u32 count = 0;
    while(passes[count].fn)
        count++;

    for(u32 i = 0; i < count; i++)
    {
        if(!global.silent)
        {
            LOG_OK_F("%s" COLOR_BOLD_GREEN "  Optimizing" COLOR_RESET " (%d/%d): %s", i ? "\33[2K\r" : "", i + 1, count, passes[i].description);
            fflush(OUTPUT_STREAM);
        }
        passes[i].fn(ast);
    }

    if(count)
        fprintf(OUTPUT_STREAM, "\n");
}

void eliminate_dead_code(ASTProg_T* ast)
{
}