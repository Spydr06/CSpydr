#include <stdio.h>
#include <string.h>

#include "flags.h"
#include "io.h"
#include "log.h"
#include "version.h"

#include "transpiler/transpiler.h"
#include "compiler/cppBindings.h"
#include "bytecode/BCCompiler.h"
#include "core/errors/errorHandler.h"

#include "core/parser/parser.h"
#include "core/parser/ACT.h"
#include "core/parser/ASTWalker.h"

#ifndef __linux__
#error "CSpydr currently only supports x86 linux!"
#endif

#ifdef _WIN32
    #define DEFAULT_OUTPUT_FILE "a.exe"
#endif
#ifdef __linux__
    #define DEFAULT_OUTPUT_FILE "a.o"
#endif

#define CSPYDR_GIT_REPOSITORY "https://github.com/spydr06/cspydr.git"
#define CSPYDR_GIT_DEVELOPER "https://github.com/spydr06"

const char* infoText = COLOR_BOLD_YELLOW "** THE CSPYDR PROGRAMMING LANGUAGE COMPILER **\n" COLOR_RESET
                       COLOR_BOLD_WHITE "Version:" COLOR_RESET " %s\n"
                       COLOR_BOLD_WHITE "Build:" COLOR_RESET " %s\n"
                       "\n"
                       "CSpydr is distributed under the GNU General Public License (v3)\n"
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

void compileLLVM(char* path, char* target);

int main(int argc, char* argv[])
{
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

    compileLLVM(inputFile, outputFile);

    return 0;
}

void compileBytecode(char* path, char* target)
{
    LOG_OK_F(COLOR_BOLD_GREEN "Compiling" COLOR_RESET " \"%s\"\n", path);
    char* src = readFile(path);

    lexer_T* lexer = initLexer(src, path);
    parser_T* parser = initParser(lexer);
    AST_T* root = parserParse(parser);

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
        LOG_OK_F(COLOR_BOLD_GREEN "Compiling" COLOR_RESET " \"%s\"\n", path);
    char* src = readFile(path);

    lexer_T* lexer = initLexer(src, path);
    parser_T* parser = initParser(lexer);
    AST_T* root = parserParse(parser);

    transpiler_T* transpiler = initTranspiler();
    char* out = transpileToC(transpiler, root);
    LOG_INFO_F("%s", out);

    free(root);
    free(lexer);
    free(parser);
    free(transpiler);
}

void compileLLVM(char* path, char* target)
{
    LOG_OK_F(COLOR_BOLD_GREEN "Compiling" COLOR_RESET " \"%s\"\n", path);
    char* src = readFile(path);

    lexer_T* lexer = initLexer(src, path);
    parser_T* parser = initParser(lexer);
    AST_T* root = parserParse(parser);
    ACTRoot_T* actionTree = generateActionTree(root);

    compile(root, target, path);

    free(root);
    free(lexer);
    free(parser);
    free(actionTree);
}