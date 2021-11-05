#define __CSP_GLOBAL_OWNER
#include "globals.h"

const char* optimization_flag = "-O3";
const char* all_warnings_flag = "-Wall";

void init_globals(void)
{
    compiler_flags = init_list(sizeof(char*));

    // default compiler flags
    list_push(compiler_flags, (void*) optimization_flag);
    list_push(compiler_flags, (void*) all_warnings_flag);
}

void globals_exit_hook(void)
{
    free_list(compiler_flags);
}