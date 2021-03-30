#ifndef CSPYDR_LEXER_H
#define CSPYDR_LEXER_H

#include "token.h"
#include <stdlib.h>
#include "errors/errorHandler.h"

typedef struct LEXER_STRUCT
{
    char* src;
    char* srcPath;
    char* currentLine;
    size_t srcSize;
    char c;
    unsigned int i;
    unsigned int iInLine;
    unsigned int line;

    errorHandler_T* errorHandler;
} lexer_T;

lexer_T* initLexer(char* src, char* path);

void lexerAdvance(lexer_T* lexer);
char lexerPeek(lexer_T* lexer, int offset);

token_T* lexerConsume(lexer_T* lexer, token_T* token);
token_T* lexerConsumeType(lexer_T* lexer, tokenType_T type);

token_T* lexerNextToken(lexer_T* lexer);

#endif
