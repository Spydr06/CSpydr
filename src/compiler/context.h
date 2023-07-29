#ifndef CSPYDR_CONTEXT_H
#define CSPYDR_CONTEXT_H

#include "ast/ast.h"
#include "error/exception.h"
#include "hashmap.h"
#include "util.h"
//#include "cspydr.h"

Flags_T default_flags(void);

typedef struct CSPYDR_CONTEXT_STRUCT {
    i32 ct;
    i32 fs;

    i32 last_exit_code;
    u32 max_macro_call_depth;

    u32 emitted_warnings;
    u32 emitted_errors;

    Flags_T flags;

    struct {
        char* exec_name;
        char* main_src_file;
        char* std_path;
        char* target;
    } paths;

    struct {
        char** argv;
        i32 argc;
    } args;

    ASTObj_T** current_obj;

    List_T* compiler_flags;
    List_T* linker_flags;
    u64 total_source_lines;

    Exception_T main_error_exception;

    // list of libraries used in the [link()] directive
    HashMap_T* included_libs;

    // timesteps recorded by `timer/timer.c`
    List_T* timesteps;
} Context_T;

void init_context(Context_T* context);

#endif
