/*
    CSPC - THE CSPYDR PROGRAMMING LANGUAGE COMPILER
    This is the main file and entry point to the compiler.

    This compiler and all components of CSpydr, except external dependencies (LLVM, acutest, ...), are licensed under the MIT license.

    Creator:
        https://github.com/spydr06
    Official git repository:
        https://github.com/spydr06/cspydr.git
*/

// std includes
#include <stdlib.h>
#include <string.h>

// compiler includes
#include "io/repl/repl.h"
#include "toolchain.h"
#include "io/io.h"
#include "io/log.h"
#include "codegen/llvm/llvm_codegen.h"
#include "codegen/transpiler/c_codegen.h"
#include "platform/platform_bindings.h"
#include "version.h"
//#include "ast/xml.h"

// default texts, which get shown if you enter help, info or version flags
// links to me, the creator of CSpydr
// please be nice and don't change them without any reason. You may add yourself to the credits, if you changed something
#define CSPYDR_GIT_REPOSITORY "https://github.com/spydr06/cspydr.git"
#define CSPYDR_GIT_DEVELOPER  "https://github.com/spydr06"
#define CSPYDR_SUBREDDIT      "https://reddit.com/r/cspydr"

#define CSPC_HELP_COMMAND "cspc --help"

const char* usage_text = COLOR_BOLD_WHITE "Usage:" COLOR_RESET " cspc [run, build, debug, repl] [<input file> <flags>]\n"
                         "       cspc [--help, --info, --version]\n";

// this text gets shown if -i or --info is used
const char* info_text = COLOR_BOLD_YELLOW "** CSPC - THE CSPYDR PROGRAMMING LANGUAGE COMPILER **\n" COLOR_RESET
                       COLOR_BOLD_WHITE "Version:" COLOR_RESET " %s\n"
                       COLOR_BOLD_WHITE "Build:" COLOR_RESET " %s\n"
                       "\n"
                       "Copyright (c) 2021 Spydr06\n"
                       "CSpydr is distributed under the MIT license\n"
                       "This is free software; see the source for copying conditions;\n"
                       "you may redistribute it under the terms of the MIT license\n"
                       "This program has absolutely no warranty.\n"
                       "\n"
                       COLOR_BOLD_WHITE "    repository: " COLOR_RESET CSPYDR_GIT_REPOSITORY "\n"
                       COLOR_BOLD_WHITE "    developer:  " COLOR_RESET CSPYDR_GIT_DEVELOPER "\n"
                       "\n"
                       "Type -h or --help for help page.\n";

// this text gets shown if -h or --help is used
const char* help_text = "%s"
                       COLOR_BOLD_WHITE "Actions:\n" COLOR_RESET
                       "  build    Builds a cspydr program to a binary to execute.\n"
                       "  run      Builds, then runs a cspydr program directly.\n"
                       "  debug    Runs a cspydr program with special debug tools. [!!NOT IMPLEMENTED YET!!]\n"
                       "  repl     Opens the cspydr REPL console.\n"
                       COLOR_BOLD_WHITE "Options:\n" COLOR_RESET
                       "  -h, --help             Displays this help text and quits.\n"
                       "  -v, --version          Displays the version of CSpydr and quits.\n"
                       "  -i, --info             Displays information text and quits.\n"
                       "  -o, --output [file]    Sets the target output file (default: " DEFAULT_OUTPUT_FILE ").\n"
                       "  -t, --transpile        Instructs the compiler to compile to C source code.\n"
                       "  -l, --llvm             Instructs the compiler to compile to LLVM BitCode (default).\n"
                       "      --print-code       Prints the generated code (C | Assembly | LLVM IR).\n"
                       "      --silent           Disables all command line output except error messages.\n"
                       "      --cc [compiler]    Sets the C compiler being used after transpiling (default: " DEFAULT_CC ")\n"
                       "      --cc-flags [flags] Sets the C compiler flags, must be last argument (default: " DEFAULT_CC_FLAGS ")\n"
                       "      --from-xml         Instructs the compiler to construct a AST directly from a XML file (debug!!)\n"
                       "      --to-xml           Instructs the compiler to parse the AST to a XML file (debug!!)\n"
                       "\n"
                       "If you are unsure, what CSpydr is (or how to use it), please check out the GitHub repository: \n" CSPYDR_GIT_REPOSITORY "\n"
                       /*"Help and community support: " CSPYDR_SUBREDDIT ".\n"*/;

// this text gets shown if -v or --version is used
const char* version_text = COLOR_BOLD_YELLOW "** THE CSPYDR PROGRAMMING LANGUAGE COMPILER **\n" COLOR_RESET
                          COLOR_BOLD_WHITE "Version:" COLOR_RESET " %s\n"
                          COLOR_BOLD_WHITE "Build:" COLOR_RESET " %s\n"
                          "\n"
                          "For more information type -i.\n";

const struct { char* as_str; Action_T ac; } action_table[AC_UNDEF] = {
    {"build", AC_BUILD},
    {"run",   AC_RUN},
    {"debug", AC_DEBUG},
    {"repl",  AC_REPL},
};

static inline bool streq(char* a, char* b)
{
    return strcmp(a, b) == 0;
}

static void evaluate_info_flags(char* argv)
{
    char csp_build[32];
    get_cspydr_build(csp_build);
    
    if(streq(argv, "-h") || streq(argv, "--help"))
        printf(help_text, usage_text);
    else if(streq(argv, "-i") || streq(argv, "--info"))
        printf(info_text, get_cspydr_version(), csp_build);
    else if(streq(argv, "-v") || streq(argv, "--version"))
        printf(version_text, get_cspydr_version(), csp_build);
    else
    {
        LOG_ERROR_F("unknown or wrong used flag \"%s\", type \"cspydr --help\" to get help.", argv);
        exit(1);
    }

    exit(0);
}

// entry point
i32 main(i32 argc, char* argv[])
{
    init_globals();
    atexit(globals_exit_hook);
    atexit(llvm_exit_hook);

    exec_name = argv[0]; // save the execution name for later use
    if(argc == 1)
    {
        LOG_ERROR_F("[Error] Too few arguments given.\n" COLOR_RESET "%s", usage_text);
        exit(1);
    }

    // if there are 2 args, check for --help, --info or --version flags
    if(argv[1][0] == '-')
        evaluate_info_flags(argv[1]);

    // get the action to perform
    Action_T action = AC_UNDEF;
    ct = DEFAULT_COMPILE_TYPE;
    for(i32 i = 0; i < AC_UNDEF; i++)
        if(streq(argv[1], action_table[i].as_str))
            action = action_table[i].ac;
    if(action == AC_UNDEF)
    {
        LOG_ERROR_F("[Error] Unknown action \"%s\", expect [build, run, debug, repl]\n", argv[1]);
    }
    
    // declare the input/output files
    char* output_file = DEFAULT_OUTPUT_FILE;
    char* input_file;

    if(action == AC_REPL)
    {
        // remove the first two flags form argc/argv
        argc -= 2;
        argv += 2;
    }
    else
    {
        input_file = argv[2];
        if(!file_exists(input_file))
        {
            LOG_ERROR_F("[Error] Error opening file \"%s\": No such file or directory\n", input_file);
            exit(1);
        }

        // remove the first three flags form argc/argv
        argc -= 3;
        argv += 3;
    }

    // get all the other flags
    for(i32 i = 0; i < argc; i++)
    {
        char* arg = argv[i];

        if(streq(arg, "-o") || streq(arg, "--output"))
        {
            if(!argv[++i])
            {
                LOG_ERROR("[Error] Expect target file path after -o/--output.\n");
                exit(1);
            }
            output_file = argv[i];
        }
        else if(streq(arg, "--print-code"))
            print_code = true;
        else if(streq(arg, "-t") || streq(arg, "--transpile"))
            ct = CT_TRANSPILE;
        else if(streq(arg, "-l") || streq(arg, "--llvm"))
            ct = CT_LLVM;
        else if(streq(arg, "-a") || streq(arg, "--asm"))
            ct = CT_ASM;
        else if(streq(arg, "--silent"))
            silent = true;
        else if(streq(arg, "--cc"))
        {
            if(!argv[++i])
            {
                LOG_ERROR("[Error] Expect C compiler name after --cc.\n");
                exit(1);
            }
            cc = argv[i];
        }
        else if(streq(arg, "--cc-flags"))
        {
            for(i++; i < argc; i++)
                list_push(compiler_flags, argv[i]);
            break;
        }
        else if(streq(arg, "--to-xml"))
            ct = CT_TO_XML;
        else
            evaluate_info_flags(argv[i]);
    }

    if(action == AC_REPL)
        repl();
    else
        compile(input_file, output_file, action);

    if(!streq(cc_flags, DEFAULT_CC_FLAGS))
        free(cc_flags);

    return 0;
}