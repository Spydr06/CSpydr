#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Types.h>
#include <stdio.h>
#include <string.h>
#include "input.h"
#include "log.h"
#include "core/parser.h"
#include "bytecode/compiler.h"
#include "llvm/compiler.h"
#include "core/errors/errorHandler.h"

#include "llvm/llvm.h"

#define CSPYDR_VERSION "v0.0.1"

void compileFile(char* path);

int main(int argc, char* argv[])
{
    LOG_WARN("** THE CSPYDR LANGUAGE COMPILER %s **\n", CSPYDR_VERSION);

    if(argc < 2) 
    {
        LOG_ERROR("Please specify input file.%s", "\n");
        return -1;
    }

    compileFile(argv[1]);

    return 0;
}

void compileFile(char* path)
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

    //compileProgram(root, "test.bc", path);
exit_routine:   //label called by the error handler to safely exit out of the program
    free(root);
    free(lexer);
    free(compiler);
    free(parser);

    //token_T* token;
    /*while((token = lexerNextToken(lexer))->type != TOKEN_EOF)
    {
        LOG_INFO("%s\n", tokenToString(token));
    }*/
}