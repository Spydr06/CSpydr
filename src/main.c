#include "bytecode/BCCompiler.h"

#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Types.h>
#include <stdio.h>
#include <string.h>
#include "flags.h"
#include "input.h"
#include "log.h"
#include "core/parser.h"
#include "core/errors/errorHandler.h"
#include "transpiler/transpiler.h"

#include "compiler/cppBindings.h"

#define CSPYDR_VERSION "v0.0.1"

#ifndef __linux__
#error "CSpydr currently only supports x86 linux!"
#endif

#ifdef _WIN32
    #define DEFAULT_OUTPUT_FILE "a.exe"
#endif
#ifdef __linux__
    #define DEFAULT_OUTPUT_FILE "a.o"
#endif

const char* helpText = COLOR_BOLD_WHITE "usage:" COLOR_RESET " cspydr [options] source files [options]\n"
                       COLOR_BOLD_WHITE "options:\n" COLOR_RESET
                       "  -h, -help\t\tdisplays this help text and quits.\n"
                       "  -v, -version\t\tdisplays the version of CSpydr and quits.\n"
                       "  -o, -output [file]\tset the target output file (default: " DEFAULT_OUTPUT_FILE ")\n"
                       "  -d, -debug\t\tenable debug output.\n";

void compileFile(char* path, char* target);

int main(int argc, char* argv[])
{
    LOG_WARN("** THE CSPYDR LANGUAGE COMPILER %s **\n", CSPYDR_VERSION);

    char* inputFile = NULL;
    char* outputFile = DEFAULT_OUTPUT_FILE;

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
                printf("CSpydr version %s", CSPYDR_VERSION);
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
            default:
                LOG_ERROR("Unknown flag '%d'. Type -h for help.\n", currentFlag->type);
                break;
        }
    }

    if(inputFile == NULL)
    {
        LOG_ERROR("Must define input file. Type -h for help.%s", "\n");
        return 0;
    }

    compileFile(inputFile, outputFile);

    return 0;
}

void compileBytecode(char* path, char* target)
{
    LOG_OK(COLOR_BOLD_GREEN "Compiling" COLOR_RESET " \"%s\"\n", path);
    char* src = readFile(path);

    lexer_T* lexer = initLexer(src, path);
    parser_T* parser = initParser(lexer);
    //validator_T* validator = initASTValidator();
    AST_T* root = parserParse(parser);
    //validateAST(validator, root);

    BCCompiler_T* compiler = initBCCompiler();
    compileBC(compiler, root);

    for(int i = 0; i < compiler->instructions->size; i++)
    {
        printf("%s\n", BCInstructionToString((BCInstruction_T*) compiler->instructions->items[i]));
    }

    free(root);
    free(lexer);
    free(compiler);
    free(parser);
}

void compileTranspiling(char* path, char* target)
{
        LOG_OK(COLOR_BOLD_GREEN "Compiling" COLOR_RESET " \"%s\"\n", path);
    char* src = readFile(path);

    lexer_T* lexer = initLexer(src, path);
    parser_T* parser = initParser(lexer);
    //validator_T* validator = initASTValidator();
    AST_T* root = parserParse(parser);
    //validateAST(validator, root);

    transpiler_T* transpiler = initTranspiler();
    char* out = transpileToC(transpiler, root);
    LOG_INFO("%s", out);

    free(root);
    free(lexer);
    free(parser);
    free(transpiler);
}

void compileFile(char* path, char* target)
{
    LOG_OK(COLOR_BOLD_GREEN "Compiling" COLOR_RESET " \"%s\"\n", path);
    char* src = readFile(path);

    lexer_T* lexer = initLexer(src, path);
    parser_T* parser = initParser(lexer);
    //validator_T* validator = initASTValidator();
    AST_T* root = parserParse(parser);
    //validateAST(validator, root);

    compile(root, target);

    free(root);
    free(lexer);
    free(parser);
}