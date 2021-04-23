#include <stdio.h>
#include <string.h>

#include "flags.h"
#include "io.h"
#include "log.h"
#include "version.h"

#include "compiler/cppBindings.h"
#include "core/errors/errorHandler.h"

#include "core/parser/parser.h"
#include "core/parser/AST.h"

#include "transpiler/transpiler.h"

#ifndef __linux__
#error "CSpydr currently only supports x86 linux!"
#endif

#ifdef _WIN32
    #include <windows.h>

    #define DEFAULT_OUTPUT_FILE "a.exe"
#endif
#ifdef __linux__
    #include <linux/limits.h>

    #define DEFAULT_OUTPUT_FILE "a.o"
#endif

#define CSPYDR_GIT_REPOSITORY "https://github.com/spydr06/cspydr.git"
#define CSPYDR_GIT_DEVELOPER "https://github.com/spydr06"

const char* infoText = COLOR_BOLD_YELLOW "** THE CSPYDR PROGRAMMING LANGUAGE COMPILER **\n" COLOR_RESET
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

const char* helpText = COLOR_BOLD_WHITE "usage:" COLOR_RESET " cspydr [options] source files [options]\n"
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

const char* versionText = COLOR_BOLD_YELLOW "** THE CSPYDR PROGRAMMING LANGUAGE COMPILER **\n" COLOR_RESET
                          COLOR_BOLD_WHITE "Version:" COLOR_RESET " %s\n"
                          COLOR_BOLD_WHITE "Build:" COLOR_RESET " %s\n"
                          "\n"
                          "For more information type -i.\n";

const char* getCSpydrVersion();
const char* getCSpydrBuild();
static char* getAbsoluteStdPath(char* relativePath);
void compileLLVM(char* path, char* target);
void compileTranspiling(char* path, char* target);

typedef enum COMPILE_TYPE_ENUM
{
    COMPILE_LLVM, COMPILE_TRANSPILING
} compileType_T;

int main(int argc, char* argv[])
{
    char* inputFile = NULL;
    char* outputFile = DEFAULT_OUTPUT_FILE;

    compileType_T compileType = COMPILE_LLVM;

    flagDispatcher_T* dispatcher = dispatchFlags(argc, argv);
    for(int i = 0; i < dispatcher->flags->size; i++)
    {
        flag_T* currentFlag = dispatcher->flags->items[i];

        switch(currentFlag->type)
        {
            case FLAG_HELP:
                printf("%s", helpText);
                return 0;
            case FLAG_VERSION:
                printf(versionText, getCSpydrVersion(), getCSpydrBuild());
                return 0;
            case FLAG_OUTPUT:
                outputFile = calloc(strlen(currentFlag->value) + 1, sizeof(char));
                strcpy(outputFile, currentFlag->value);
                break;
            case FLAG_INPUT:
                inputFile = calloc(strlen(currentFlag->value) + 1, sizeof(char));
                strcpy(inputFile, currentFlag->value);
                break;
            case FLAG_ENABLE_TRANSPILING:
                compileType = COMPILE_TRANSPILING;
                break;
            case FLAG_DEBUG:
                //TODO: create a global debugging flag
                break;
            case FLAG_INFO:
                printf(infoText, getCSpydrVersion(), getCSpydrBuild());
                return 0;
            default:
                LOG_ERROR_F("Unknown flag '%d'. Type -h for help.\n", currentFlag->type);
                break;
        }
    }

    if(inputFile == NULL)
    {
        LOG_ERROR("Must define input file. Type -h for help.\n");
        return 0;
    }

    switch(compileType)
    {
        case COMPILE_LLVM:
            compileLLVM(inputFile, outputFile);
            break;
        case COMPILE_TRANSPILING:
            compileTranspiling(inputFile, outputFile);
            break;
        default:
            LOG_ERROR_F("Unknown compile type \"%d\"\n", compileType);
            break;
    }

    return 0;
}

void compileLLVM(char* path, char* target)
{
    LOG_OK_F(COLOR_BOLD_GREEN "Compiling" COLOR_RESET " \"%s\"\n", path);
    char* src = readFile(path);

    lexer_T* lexer = initLexer(src, path);
    parser_T* parser = initParser(lexer);
    ASTRoot_T* root = parserParse(parser);

    compile(root, target, path);

    freeAST(root);

    free(root);
    free(lexer);
    free(parser);
}

void compileTranspiling(char* path, char* target)
{
    LOG_OK_F(COLOR_BOLD_GREEN "Compiling" COLOR_RESET " \"%s\"\n", path);
    char* src = readFile(path);

    lexer_T* lexer = initLexer(src, path);
    parser_T* parser = initParser(lexer);
    ASTRoot_T* root = parserParse(parser);

    transpiler_T* transpiler = initTranspiler(getAbsoluteStdPath("std/impl/csp_std.cpp"));
    transpileAST(root, transpiler);
    char* outputCode = emitCode(transpiler);

#if defined(__linux__)
    sh("mkdir -p .cache");
#elif defined(_WIN32)
    //TODO
#endif

    writeFile(".cache/tmp.cpp", outputCode);

    printf("%s\n", outputCode);
    free(outputCode);

    freeAST(root);

    free(transpiler);
    free(lexer);
    free(parser);
}

static char* getAbsoluteStdPath(char* relativePath)
{
#ifdef __linux__
    char* absolutePath = calloc(PATH_MAX, sizeof(char));
    realpath(relativePath, absolutePath);
    return absolutePath;
#endif
}


