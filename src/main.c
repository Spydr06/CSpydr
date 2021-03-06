#include <stdio.h>
#include <string.h>
#include "input.h"
#include "log.h"
#include "parser.h"

#define VERSION "v0.0.1"

void compileFile(char* path);

int main(int argc, char* argv[])
{
    LOG_WARN("** THE CSPYDR LANGUAGE COMPILER %s **\n", VERSION);

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
    LOG_OK("Compiling" RESET " \"%s\"\n", path);
    char* src = readFile(path);

    lexer_T* lexer = initLexer(src);
    parser_T* parser = initParser(lexer);

    parserParse(parser);

    //token_T* token;
    /*while((token = lexerNextToken(lexer))->type != TOKEN_EOF)
    {
        LOG_INFO("%s\n", tokenToString(token));
    }*/

    //sh("cc a.c");
    //sh("rm -rf a.c");
}