#include <stdio.h>
#include "input.h"
#include "parser.h"

void compileFile(char* path);

int main(int argc, char* argv[])
{
    printf("** THE CSPYDR LANGUAGE COMPILER **\n");

    if(argc < 2) 
    {
        fprintf(stderr, "[ERROR] please specify input file!\n");
        return -1;
    }

    compileFile(argv[1]);

    return 0;
}

void compileFile(char* path)
{
    printf("Compiling file '%s'\n", path);
    char* src = readFile(path);

    printf("%s\n", src);

    lexer_T* lexer = initLexer(src);
    parser_T* parser = initParser(lexer);

    parserParse(parser);

    /*while((token = lexerNextToken(lexer))->type != TOKEN_EOF)
    {
        printf("%s\n", tokenToString(token));
    }*/
}