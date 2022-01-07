#define __CSP_GLOBAL_OWNER
#include "globals.h"

#include <string.h>

const char* optimization_flag = "-O3";
const char* all_warnings_flag = "-Wall";

void init_globals(void)
{
    memset(&global, 0, sizeof(global));
    global.compiler_flags = init_list(sizeof(char*));

    // default compiler flags
    list_push(global.compiler_flags, (void*) optimization_flag);
    list_push(global.compiler_flags, (void*) all_warnings_flag);

    global.linker_flags = init_list(sizeof(char*));
}

void globals_exit_hook(void)
{
    free_list(global.compiler_flags);
    free_list(global.linker_flags);
}