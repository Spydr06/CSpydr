#ifndef CSPYDR_GLOBALS_H
#define CSPYDR_GLOBALS_H

#ifdef __CSP_GLOBAL_OWNER
    #define __CSP_GLOBAL
#else
    #define __CSP_GLOBAL extern
#endif

#include <stdbool.h>

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

#endif