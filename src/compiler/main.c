/*
    THE CSPYDR PROGRAMMING LANGUAGE COMPILER
    This is the main file and entry point to the compiler.

    This compiler and all components of CSpydr, except LLVM, are licensed under the GNU General Public License v3.0.

    Creator:
        https://github.com/spydr06
    Official git repository:
        https://github.com/spydr06/cspydr.git
*/

// std includes
#include <stdio.h>
#include <string.h>

// compiler includes
#include "ast/ast.h"
#include "io/file.h"
#include "io/flags.h"
#include "io/io.h"
#include "io/log.h"
#include "list.h"
#include "version.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "parser/preprocessor.h"
#include "error/errorHandler.h"
#include "platform/platform_bindings.h"
#include "codegen/llvm/llvm_codegen.h"

// default texts, which get shown if you enter help, info or version flags
// links to me, the creator of CSpydr
// please be nice and don't change them
#define CSPYDR_GIT_REPOSITORY "https://github.com/spydr06/cspydr.git"
#define CSPYDR_GIT_DEVELOPER "https://github.com/spydr06"

// this text gets shown if -i or --info is used
const char* info_text = COLOR_BOLD_YELLOW "** THE CSPYDR PROGRAMMING LANGUAGE COMPILER **\n" COLOR_RESET
                       COLOR_BOLD_WHITE "Version:" COLOR_RESET " %s\n"
                       COLOR_BOLD_WHITE "Build:" COLOR_RESET " %s\n"
                       "\n"
                       "Copyright (C) 2021 Spydr06\n"
                       "CSpydr is distributed under the GNU General Public License (v3)\n"
                       "This is free software; see the source for copying conditions;\n"
                       "you may redistribute it under the terms of the GNU GPL version 3\n"
                       "or (at your option) a later version.\n"
                       "This program has absolutely no warranty.\n"
                       "\n"
                       COLOR_BOLD_WHITE "    repository: " COLOR_RESET CSPYDR_GIT_REPOSITORY "\n"
                       COLOR_BOLD_WHITE "    developer:  " COLOR_RESET CSPYDR_GIT_DEVELOPER "\n"
                       "\n"
                       "Type -h or --help for help page.\n";

// this text gets shown if -h or --help is used
const char* help_text = COLOR_BOLD_WHITE "usage:" COLOR_RESET " cspydr [options] source files [options]\n"
                       COLOR_BOLD_WHITE "options:\n" COLOR_RESET
                       "  -h, --help\t\tdisplays this help text and quits.\n"
                       "  -v, --version\t\tdisplays the version of CSpydr and quits.\n"
                       "  -i, --info\t\tdisplays information text and quits.\n"
                       "  -o, --output [file]\tset the target output file (default: " DEFAULT_OUTPUT_FILE ")\n"
                       "  -t, --transpile\tsets the compile type to transpile to c++,\n"
                       "                   \tthen compile (default: compile to LLVM IR)\n"
                       "  -d, --debug\t\tenable debug output.\n"
                       "\n"
                       "If you are unsure, what CSpydr is, please check out the GitHub repository: \n" CSPYDR_GIT_REPOSITORY "\n";

// this text gets shown if -v or --version is used
const char* version_text = COLOR_BOLD_YELLOW "** THE CSPYDR PROGRAMMING LANGUAGE COMPILER **\n" COLOR_RESET
                          COLOR_BOLD_WHITE "Version:" COLOR_RESET " %s\n"
                          COLOR_BOLD_WHITE "Build:" COLOR_RESET " %s\n"
                          "\n"
                          "For more information type -i.\n";

// declaration of the functions used below
extern const char* get_cspydr_version();
extern const char* get_cspydr_build();
extern void compile_llvm(char* path, char* target);

// entry point
int main(int argc, char* argv[])
{
    // declare the input/output files
    char* inputFile = NULL;
    char* outputFile = DEFAULT_OUTPUT_FILE;

    // dispatch all given flags
    FlagDispatcher_T* dispatcher = dispatch_flags(argc, argv);
    for(int i = 0; i < dispatcher->flags->size; i++)
    {
        Flag_T* currentFlag = dispatcher->flags->items[i];

        switch(currentFlag->type)
        {
            case FLAG_HELP:
                printf("%s", help_text);
                return 0;
            case FLAG_VERSION:
                printf(version_text, get_cspydr_version(), get_cspydr_version());
                return 0;
            case FLAG_OUTPUT:
                outputFile = calloc(strlen(currentFlag->value) + 1, sizeof(char));
                strcpy(outputFile, currentFlag->value);
                break;
            case FLAG_INPUT:
                inputFile = calloc(strlen(currentFlag->value) + 1, sizeof(char));
                strcpy(inputFile, currentFlag->value);
                break;
            case FLAG_DEBUG:
                //TODO: create a global debugging flag
                break;
            case FLAG_INFO:
                printf(info_text, get_cspydr_version(), get_cspydr_build());
                return 0;
            default:
                LOG_ERROR_F("Unknown flag '%d'. Type -h for help.\n", currentFlag->type);
                break;
        }
    }

    free_flagdispatcher(dispatcher);

    // check if an input file was given
    if(inputFile == NULL)
    {
        LOG_ERROR("Must define input file. Type -h for help.\n");
        return 0;
    }

    compile_llvm(inputFile, outputFile);

    free(inputFile);

    if(strcmp(outputFile, DEFAULT_OUTPUT_FILE) != 0) {
        free(outputFile);
    }

    return 0;
}

// sets up and runs the compilation pipeline using LLVM
void compile_llvm(char* path, char* target)
{
    SrcFile_T* main_file = read_file(path);

    ErrorHandler_T* eh = init_errorhandler(main_file);

    ASTProg_T* ast = parse_file(eh, init_list(sizeof(char*)), main_file);
    List_T* imports = ast->imports;

    for(size_t i = 0; i < imports->size; i++)
    {
        SrcFile_T* import_file = read_file(imports->items[i]);
        eh->file = import_file;

        ASTProg_T* ast = parse_file(eh, imports, import_file);
        // TODO: merge ASTProg_Ts together

        free_srcfile(import_file);
    }

    preprocess(eh, ast);

    LLVMCodegenData_T* cg = init_llvm_cg(ast, target);
    cg->print_ll = true;
    llvm_gen_code(cg);
    free_llvm_cg(cg);

    free_errorhandler(eh);
    free_srcfile(main_file);
}