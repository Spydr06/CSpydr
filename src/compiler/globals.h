#ifndef CSPYDR_GLOBALS_H
#define CSPYDR_GLOBALS_H

#include "ast/ast.h"

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
__CSP_GLOBAL struct {
    i32 ct;
    i32 fs;

    i32 last_exit_code;

    u32 max_macro_call_depth;

    union {
        struct {
            bool silent           : 1;
            bool print_code       : 1;
            bool optimize         : 1;
            bool embed_debug_info : 1;
            bool from_json        : 1;
            bool do_link          : 1;
            bool do_assemble      : 1;
            bool do_parsing       : 1;
        };
        u8 flags;
    };

    u32 emitted_warnings;
    u32 emitted_errors;

    char* exec_name;
    char* main_src_file;

    ASTObj_T** current_fn;

    List_T* compiler_flags;
    List_T* linker_flags;
} global;

__CSP_GLOBAL void globals_exit_hook(void);
__CSP_GLOBAL void init_globals(void);

#endif