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
#include "config.h"

// all global variables, that the compiler needs
struct {
    i32 ct;
    i32 fs;

    i32 last_exit_code;

    u32 max_macro_call_depth;

    bool silent;
    bool print_code;
    bool optimize;
    bool embed_debug_info;
    bool from_json;

    u32 emitted_warnings;
    u32 emitted_errors;

    char* exec_name;
    char* main_src_file;

    List_T* compiler_flags;
    List_T* linker_flags;
} __CSP_GLOBAL global;

__CSP_GLOBAL void globals_exit_hook(void);
__CSP_GLOBAL void init_globals(void);

#endif