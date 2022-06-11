#include "api.h"
#include "include/cspydr.h"
#include <stdlib.h>
#include <memory.h>

Compiler_T* init_compiler() 
{
    Compiler_T* compiler = malloc(sizeof(struct CSPYDR_COMPILER_STRUCT));
    memset(compiler, 0, sizeof(struct CSPYDR_COMPILER_STRUCT));

    compiler->initialized = true;
    return compiler;
}

void free_compiler(Compiler_T *compiler)
{
    free(compiler);
}