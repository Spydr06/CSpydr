/*
    CSP-LINT - THE CSPYDR PROGRAMMING LANGUAGE LINTER
    This is the main file and entry point to the linter.

    This linter, the compiler and all components of CSpydr, except external dependencies (acutest, json-c), are licensed under the MIT license.

    Creator:
        https://github.com/spydr06
    Official git repository:
        https://github.com/spydr06/cspydr.git
*/

// std includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// compiler includes
#include <io/log.h>
#include <globals.h>
#include <version.h>

// linter includes
#include "error/error.h"
#include "io/io.h"
#include "linter.h"
#include "error.h"

#define streq(a, b) (strcmp(a, b) == 0)

// default texts, which get shown if you enter help, info or version flags
#define CSPL_HELP_COMMAND "csp-lint --help"

const char usage_text[] = COLOR_BOLD_WHITE "Usage:" COLOR_RESET " csp-lint <input file> <flags>\n";

// this text gets shown if -i or --info is used
const char info_text[] = COLOR_BOLD_MAGENTA "** csp-lint - The CSpydr Programming Language Linter **\n" COLOR_RESET
                       COLOR_BOLD_WHITE "Version:" COLOR_RESET " %s\n"
                       COLOR_BOLD_WHITE "Build:" COLOR_RESET " %s\n"
                       "\n"
                       "Copyright (c) 2022 Spydr06\n"
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

const char help_text[] = "%s"
                         COLOR_BOLD_WHITE "Options:\n" COLOR_RESET
                         "  -h, --help             | Displays this help text and quits\n"
                         "  -i, --info             | Displays information text and quits\n"
                         "      --version          | Displays the version of CSpydr and quits\n"
                         "  -v, --verbose          | Sets verbose error messages, used for programs\n"
                         "                         | communicating with the linter\n"
                         "  -o, --output           | Sets an output file for the error log\n";

// this text gets shown if -v or --version is used
const char version_text[] = COLOR_BOLD_MAGENTA "** csp-lint - The CSpydr Programming Language Linter **\n" COLOR_RESET
                            COLOR_BOLD_WHITE "Version:" COLOR_RESET " %s\n"
                            COLOR_BOLD_WHITE "Build:" COLOR_RESET " %s\n"
                            "\n"
                            "For more information type -i; for help type -h.\n";


i32 main(i32 argc, char* argv[]) {
    if(argc == 1)
    {
        LOG_ERROR_F("[Error] Too few arguments given.\n" COLOR_RESET "%s", usage_text);
        exit(1);
    }

    set_error_output_file(stderr);

    init_globals();
    atexit(globals_exit_hook);
    global.exec_name = argv[0]; // save the execution name for later use

    char* src_path = NULL;
    
    for(i32 i = 1; i < argc; i++)
    {
        char* arg = argv[i];

        if(streq(arg, "-h") || streq(arg, "--help"))
        {
            printf(help_text, usage_text);
            exit(0);
        }
        else if(streq(arg, "--version"))
        {
            char buf[BUFSIZ] = {};
            get_cspydr_build(buf);
            printf(version_text, get_cspydr_version(), buf);
            exit(0);
        } 
        else if(streq(arg, "-i") || streq(arg, "--info"))
        {
            char buf[BUFSIZ] = {};
            get_cspydr_build(buf);
            printf(info_text, get_cspydr_version(), buf);
            exit(0);
        }
        else if(streq(arg, "-v") || streq(arg, "--verbose"))
            set_error_handler(linter_error_handler);
        else if(streq(arg, "-o") || streq(arg, "--output"))
        {
            char* path = argv[++i];
            if(!path)
            {
                LOG_ERROR_F("expect file name after `%s`\n", arg);
                exit(1);
            }
                
            FILE* output_file = open_file(path);
            set_error_output_file(output_file);
            atexit(close_output_file);

            set_error_handler(linter_error_handler); // switch to the linter error handler since text files don't support color
        }
        else if(arg[0] == '-')
        {
            LOG_ERROR_F("[Error] Invalid option `%s`\n", arg);
            exit(1);
        }
        else
        {
            if(src_path)
            {
                LOG_ERROR("[Error] More than one file path given\n");
                exit(1);
            }
            src_path = arg;
        }
    }

    if(!src_path)
    {
        LOG_ERROR("[Error] No file path given\n");
        exit(1);
    }

    atexit(summary);

    return lint(src_path);
}