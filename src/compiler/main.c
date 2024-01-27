/*
    CSPC - THE CSPYDR PROGRAMMING LANGUAGE COMPILER
    This is the main file and entry point to the compiler.

    This compiler and all components of CSpydr, except external dependencies (acutest, json-c), are licensed under the MIT license.

    Creator:
        https://github.com/spydr06
    Official git repository:
        https://github.com/spydr06/cspydr.git
*/

/*
    The MIT License (MIT) - This License applies to all parts of CSpydr (cspc, csp-lint, libcspydr and the STD library)

    Copyright (c) 2021 - 2023 Spydr06

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

// std includes
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// compiler includes
#include "platform/linux/linux_platform.h"
#include "timer/timer.h"
#include "toolchain.h"
#include "io/io.h"
#include "io/log.h"
#include "codegen/transpiler/c_codegen.h"
#include "platform/platform_bindings.h"
#include "util.h"
#include "version.h"
#include "config.h"
#include "context.h"
#include "debugger/debugger.h"

#ifdef CSPYDR_USE_LLVM
    #include "codegen/llvm/llvm_codegen.h"
#endif

#define streq(a, b) (strcmp(a, b) == 0)

// default texts, which get shown if you enter help, info or version flags
#define CSPC_HELP_COMMAND "cspc --help"

typedef enum ACTION_ENUM
{
    AC_NULL = -1,
    AC_BUILD,
    AC_RUN,
    AC_LIB,
    AC_DEBUG,
    AC_SHELL,
} Action_T;

const char usage_text[] = COLOR_BOLD_WHITE "Usage:" COLOR_RESET " cspc [run, build, debug] [<input file> <flags>]\n"
                         "       cspc [--help, --info, --version]\n";

// this text gets shown if -i or --info is used
const char info_text[] = COLOR_BOLD_MAGENTA " ** cspc - The CSpydr Programming Language Compiler **\n" COLOR_RESET
                       COLOR_BOLD_WHITE "Version:" COLOR_RESET " %s\n"
                       COLOR_BOLD_WHITE "Build:" COLOR_RESET " %s\n"
                       "\n"
                       "Copyright (c) 2021 - 2022 Spydr06\n"
                       "CSpydr is distributed under the MIT license.\n"
                       "This is free software; see the source for copying conditions;\n"
                       "you may redistribute it under the terms of the MIT license.\n"
                       "This program has absolutely no warranty.\n"
                       "\n"
                    #ifdef CSPYDR_SHOW_GIT_REPOSITORY
                       COLOR_BOLD_WHITE "    repository:     " COLOR_RESET CSPYDR_GIT_REPOSITORY "\n"
                    #endif
                    #ifdef CSPYDR_SHOW_GIT_DEVELOPER
                       COLOR_BOLD_WHITE "    developer:      " COLOR_RESET CSPYDR_GIT_DEVELOPER "\n"
                    #endif
                    #ifdef CSPYDR_SHOW_SUBREDDIT
                       COLOR_BOLD_WHITE "    support & help: " COLOR_RESET CSPYDR_SUBREDDIT "\n"
                    #endif
                       "\n"
                       "Type -h or --help for the help page.\n";

// this text gets shown if -h or --help is used
const char help_text[] = "%s"
                       COLOR_BOLD_WHITE "Actions:\n" COLOR_RESET
                       "  build    Builds a cspydr program to a binary to execute.\n"
                       "  run      Builds, then runs a cspydr program directly.\n"
                       "  debug    Builds a cspydr program, then launches the debugger shell.\n"
                       "  lib      Builds a static or shared library.\n"
                       COLOR_BOLD_WHITE "Options:\n" COLOR_RESET
                       "  -h, --help                | Displays this help text and quits\n"
                       "  -v, --version             | Displays the version of CSpydr and quits\n"
                       "  -i, --info                | Displays information text and quits\n"
                       "  -o, --output [file]       | Sets the target output file\n"
                       "  -b, --backend [backend]   | Instructs the compiler to use a certain backend\n"
                       "                            | Options: [C, assembly, "
#ifdef CSPYDR_USE_LLVM
    "llvm, "
#endif          
                                                    "interpreter, json]\n"
                       "                            | Default: `assembly`\n"
                       "      --print-code          | Prints the generated code (C | Assembly | LLVM IR)\n"
                       "      --silent              | Disables all command line output except error messages\n"
                       "      --cc [compiler]       | Sets the C compiler being used after transpiling (default: " DEFAULT_CC ")\n"
                       "  -S                        | Comple only; do not assemble or link\n"
                       "  -c                        | Compile and assemble, but do not link\n"
                       "  -g -g0                    | Include/Exclude debug symbols in binary\n"
                       "  -0, --no-opt              | Disables all code optimization\n"
                       "      --set-mmcd [int]      | Sets the maximum macro call depth (default: %d) (unsafe: could cause stack overflow)\n"
                       "      --show-timings        | Shows the duration the different compiler stages took\n"
                       "  -p, --std-path            | Set the path of the standard library (default: " DEFAULT_STD_PATH ")\n"
                       "      --clear-cache         | Clears the cache located at %s" DIRECTORY_DELIMS CACHE_DIR "\n"
                       "      -- [arguments...]     | Define a list of arguments passed to the program when the `run` action is selected\n"
                       "\n"
                       "If you are unsure, what CSpydr is (or how to use it), please check out the GitHub repository: \n" CSPYDR_GIT_REPOSITORY "\n";

// this text gets shown if -v or --version is used
const char version_text[] = COLOR_BOLD_YELLOW "** THE CSPYDR PROGRAMMING LANGUAGE COMPILER **\n" COLOR_RESET
                          COLOR_BOLD_WHITE "Version:" COLOR_RESET " %s\n"
                          COLOR_BOLD_WHITE "Build:" COLOR_RESET " %s\n"
                          "\n"
                          "For more information type -i; for help type -h.\n";

const struct {
    char* as_str;
    Action_T ac;
} action_table[] =
{
    {"build", AC_BUILD},
    {"run",   AC_RUN},
    {"debug", AC_DEBUG},
    {"lib",   AC_LIB},
    {"shell", AC_SHELL},
    {NULL, -1}
};

static void run(Context_T* context, char* file);
static void evaluate_info_flags(Context_T* context, char* argv);
static void store_exec_args(Context_T* context, i32 argc, char* argv[], Action_T action);
static char* default_output_file(Context_T* context, Action_T action, const char* input_file);
static CompileType_T backend_opt(const char* arg);

// entry point
i32 main(i32 argc, char* argv[])
{
    Context_T context;
    init_context(&context);

#ifdef CSPYDR_USE_LLVM
    atexit(llvm_exit_hook);
#endif

    context.paths.exec_name = argv[0]; // save the execution name for later use
    if(argc == 1)
    {
        LOG_ERROR_F(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " too few arguments given.\n" COLOR_RESET "%s", usage_text);
        exit(1);
    }

    // if there are 2 args, check for --help, --info or --version flags
    if(argv[1][0] == '-')
        evaluate_info_flags(&context, argv[1]);

    // get the action to perform

    Action_T action = AC_NULL;
    context.ct = DEFAULT_COMPILE_TYPE;
    for(i32 i = 0; action_table[i].as_str; i++)
        if(streq(argv[1], action_table[i].as_str))
            action = action_table[i].ac;
    switch(action)
    {
    case AC_NULL:
        LOG_ERROR_F(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " unknown action \"%s\", expect one of [build, run, debug, lib]\n", argv[1]);
        exit(1);
    case AC_SHELL:
        context.flags.silent = true;
        // fall through
    case AC_RUN: 
        context.flags.run_after_compile = true;
        context.flags.delete_executable = true;
        break;
    case AC_LIB:
        context.flags.require_entrypoint = false;
        break;
    default:
        break;
    }

    // declare the input files
    char* input_file = argv[2];
    if(!file_exists(input_file))
    {
        LOG_ERROR_F(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " error opening file \"%s\": No such file or directory\n", input_file);
        exit(1);
    }
    // remove the first three flags form argc/argv
    argc -= 3;
    argv += 3;

    if(action == AC_SHELL)
    {
        store_exec_args(&context, argc, argv, action); 
        argc = 0;
    }

    // get default output file
    char* output_file = default_output_file(&context, action, input_file);

    // get all the other flags
    for(i32 i = 0; i < argc; i++)
    {
        char* arg = argv[i];

        if(streq(arg, "-o") || streq(arg, "--output"))
        {
            if(!argv[++i])
            {
                LOG_ERROR(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " expect target file path after -o/--output.\n");
                exit(1);
            }
            output_file = argv[i];
        }
        else if(streq(arg, "--print-code"))
            context.flags.print_code = true;
        else if(streq(arg, "-b") || streq(arg, "--backend"))
        {
            if(!argv[++i])
            {
                LOG_ERROR(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " Expect backend name after `--backend`.\n");
                exit(1);
            }
            context.ct = backend_opt(argv[i]);
        }
        else if(streq(arg, "--from-json"))
            context.flags.from_json = true;
        else if(streq(arg, "--silent"))
            context.flags.silent = true;
        else if(streq(arg, "-S"))
            context.flags.do_linking = context.flags.do_assembling = false;
        else if(streq(arg, "-c"))
            context.flags.do_linking = false;
        else if(streq(arg, "-g"))
            context.flags.embed_debug_info = true;
        else if(streq(arg, "-g0"))
            context.flags.embed_debug_info = false;
        else if(streq(arg, "--cc"))
        {
            if(!argv[++i])
            {
                LOG_ERROR(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " Expect C compiler name after --cc.\n");
                exit(1);
            }
            context.cc = argv[i];
        }
        else if(streq(arg, "-0") || streq(arg, "--no-opt"))
            context.flags.optimize = false;
        else if(streq(arg, "--set-mmcd"))
        {
            if(!(context.max_macro_call_depth = atoi(argv[++i])))
            {
                LOG_ERROR(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " `--set-mmcd` expects an integer greater than 0\n");
                exit(1);
            }
        }
        else if(streq(arg, "--show-timings"))
            enable_timer(&context);
        else if(streq(arg, "-p") || streq(arg, "--std-path"))
        {
            if(!argv[++i])
            {
                LOG_ERROR_F(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " expect STD path after %s.", arg);
                exit(1);
            }
            context.paths.std_path = get_absolute_path(argv[i]);
        }
        else if(streq(arg, "--clear-cache"))
            context.flags.clear_cache_after = true;
        else if(streq(arg, "--"))
        {
            store_exec_args(&context, argc - i - 1, &argv[i + 1], action);
            i = argc;
        }
        else
            evaluate_info_flags(&context, argv[i]);
    }

    if(context.ct == CT_INTERPRETER && action != AC_RUN)
    {
        LOG_ERROR(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " using interpreter without code execution; aborting.\n");
        exit(1);
    }

    compile(&context, input_file, output_file);

    switch(action)
    {
        case AC_BUILD:
        case AC_LIB:
            break;
        case AC_DEBUG:
            debug_repl(&context, input_file, output_file);
            break;
        default:
            if(context.flags.run_after_compile)
                run(&context, output_file);
            break;
    }

    if(context.flags.timer_enabled)
        timer_print_summary(&context);

    if(!streq(context.cc_flags, DEFAULT_CC_FLAGS))
        free((void*) context.cc_flags);
    
    free_context(&context);

    return 0;
}

static void evaluate_info_flags(Context_T* context, char* argv)
{
    char csp_build[32];
    get_cspydr_build(csp_build);

    if(streq(argv, "-h") || streq(argv, "--help"))
        printf(help_text, usage_text, __CSP_DEFAULT_MAX_MACRO_CALL_DEPTH, get_home_directory());
    else if(streq(argv, "-i") || streq(argv, "--info"))
        printf(info_text, get_cspydr_version(), csp_build);
    else if(streq(argv, "-v") || streq(argv, "--version"))
        printf(version_text, get_cspydr_version(), csp_build);
    else if(streq(argv, "--clear-cache"))
        clear_cache(context);
    else
    {
        LOG_ERROR_F(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " unknown or wrong used flag \"%s\", type \"cspydr --help\" to get help.\n", argv);
        exit(1);
    }

    exit(0);
}

static CompileType_T backend_opt(const char* arg)
{
    char* uppercase = malloc(strlen(arg) * sizeof(char));
    for(size_t i = 0; i < strlen(arg) + 1; i++)
        uppercase[i] = toupper(arg[i]);

    CompileType_T ct;
    if(streq(uppercase, "ASSEMBLY"))
        ct = CT_ASM;
#ifdef CSPYDR_USE_LLVM
    else if(streq(uppercase, "LLVM"))
        ct = CT_LLVM;
#endif
    else if(streq(uppercase, "C") || streq(uppercase, "TRANSPILE"))
        ct = CT_TRANSPILE;
    else if(streq(uppercase, "JSON"))
        ct = CT_TO_JSON;
    else if(streq(uppercase, "INTERPRETER"))
        ct = CT_INTERPRETER;
    else
    {
        LOG_ERROR_F(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED 
            " unknown `--backend` option `%s`, expect one of [C, "
#ifdef CSPYDR_USE_LLVM
"llvm, "
#endif
            "assembly, json, interpreter]\n",
            arg
        );
        exit(1);
    }

    free(uppercase);
    return ct;
}

static void run(Context_T* context, char* filename)
{
    timer_start(context, "execution");
    if(!context->flags.do_assembling || !context->flags.do_linking) 
    {
        LOG_OK(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " cannot execute target since no executable was generated.\n");
        return;
    }

    if(!context->flags.silent)
    {
        LOG_OK_F(COLOR_BOLD_BLUE "  Executing " COLOR_RESET " %s%s", filename, context->args.argv && context->args.argc ? " [" : "\n");
        for(i32 i = 0; i < context->args.argc; i++)
            LOG_OK_F(COLOR_RESET "`%s`%s", context->args.argv[i], context->args.argc - i > 1 ? ", " : "]\n" COLOR_RESET);
    }

    const char* argv[context->args.argc + 2];
    argv[0] = filename;
    if(context->args.argc && context->args.argv != NULL)
        memcpy(argv + 1, context->args.argv, context->args.argc * sizeof(const char*));
    argv[context->args.argc + 1] = NULL;

    context->last_exit_code = subprocess(filename, (char* const*) argv, !context->flags.silent);
    timer_stop(context);
}

static void store_exec_args(Context_T* context, i32 argc, char* argv[], Action_T action) 
{
    if(!context->flags.run_after_compile)
    {
        LOG_WARN(COLOR_BOLD_YELLOW "[Warn]" COLOR_RESET COLOR_YELLOW " ignoring parameters [");
        for(i32 i = 0; i < argc; i++)
            LOG_WARN_F("`%s`%s", argv[i], argc - i > 1 ? ", " : "] after `--`, because the program is not executed.\n" COLOR_RESET);
    }

    context->args.argv = argv;
    context->args.argc = argc;
}

static char* default_output_file(Context_T* context, Action_T action, const char* input_file)
{
    const char* fileext = EXEC_FILEEXT;
    if(action == AC_LIB)
        fileext = SHARED_LIB_FILEEXT;

    uintptr_t extptr = (uintptr_t) strrchr(input_file, '.');
    if(!extptr)
        extptr = (uintptr_t) input_file + strlen(input_file);
    uintptr_t base_length = extptr - (uintptr_t) input_file;

    char* filename = calloc(base_length + strlen(fileext) + 1, sizeof(char));
    CONTEXT_ALLOC_REGISTER(context, (void*) filename);
    
    strncpy(filename, input_file, base_length);
    strcat(filename, fileext);

    return filename;
}
