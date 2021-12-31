#ifndef CSPYDR_GLOBALS_H
#define CSPYDR_GLOBALS_H

#ifdef __CSP_GLOBAL_OWNER
    #define __CSP_GLOBAL
#else
    #define __CSP_GLOBAL extern
#endif

#include <stdbool.h>
#include "util.h"
#include "list.h"

// all global variables, that the compiler needs

#define DEFAULT_COMPILE_TYPE CT_TRANSPILE
#define __CSP_MAX_TOKEN_SIZE 128
#define __CSP_MAX_FN_NUM_ARGS 128

#define __CSP_MAX_REPL_CMD_LEN (BUFSIZ * 8)

struct {
    i32 ct;
    i32 fs;

    i32 last_exit_code;

    bool silent;
    bool print_code;

    char* exec_name;
    char* main_src_file;

    List_T* compiler_flags;
} __CSP_GLOBAL global;

__CSP_GLOBAL void globals_exit_hook(void);
__CSP_GLOBAL void init_globals(void);

#endif