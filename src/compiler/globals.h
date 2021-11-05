#ifndef CSPYDR_GLOBALS_H
#define CSPYDR_GLOBALS_H

#ifdef __CSP_GLOBAL_OWNER
    #define __CSP_GLOBAL
#else
    #define __CSP_GLOBAL extern
#endif

#include <stdbool.h>
#include "list.h"

// all global variables, that the compiler needs

#define DEFAULT_COMPILE_TYPE CT_TRANSPILE
#define __CSP_MAX_TOKEN_SIZE 128
#define __CSP_MAX_FN_NUM_ARGS 128

#define __CSP_MAX_REPL_CMD_LEN (BUFSIZ * 8)

int __CSP_GLOBAL ct;
int __CSP_GLOBAL fs;

int __CSP_GLOBAL last_exit_code;

bool __CSP_GLOBAL silent;
bool __CSP_GLOBAL print_llvm;
bool __CSP_GLOBAL print_c;

char __CSP_GLOBAL* exec_name;

__CSP_GLOBAL List_T* compiler_flags;

__CSP_GLOBAL void globals_exit_hook(void);
__CSP_GLOBAL void init_globals(void);

#endif