#include "api.h"
#include "include/cspydr.h"
#include <mem/mem.h>
#include <globals.h>
#include <io/io.h>
#include <parser/parser.h>

#include <stddef.h>
#include <stdlib.h>
#include <memory.h>

Compiler_T* csp_init_compiler() 
{
    Compiler_T* compiler = malloc(sizeof(struct CSPYDR_COMPILER_STRUCT));
    memset(compiler, 0, sizeof(struct CSPYDR_COMPILER_STRUCT));

    init_globals();

    compiler->initialized = true;
    compiler->status = COMPILER_INIT;
    return compiler;
}

void csp_free_compiler(Compiler_T* compiler)
{
    globals_exit_hook();
    free(compiler);
}

CompilerStatus_T csp_get_status(Compiler_T* compiler)
{
    if(compiler)
        return compiler->status;
    else
        return COMPILER_NONE;
}

const char* csp_status_str(CompilerStatus_T status)
{
    switch(status) {
        case COMPILER_NONE:      return "NONE";
        case COMPILER_INIT:      return "INIT";
        case COMPILER_PARSED:    return "PARSED";
        case COMPILER_OPTIMIZED: return "OPTIMIZED";
        case COMPILER_GENERATED: return "GENERATED";
        case COMPILER_EXECUTED:  return "EXECUTED";
        default: return "<unknown>";
    }
}