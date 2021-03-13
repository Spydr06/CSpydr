#ifndef CSPYDR_LEXER_H
#define CSPYDR_LEXER_H

#include <stdlib.h>
#include "token.h"

typedef struct LEXER_STRUCT
{
    char* src;
    size_t srcSize;
    char c;
    unsigned int i;
} lexer_T;

lexer_T* initLexer(char* src);

void lexerAdvance(lexer_T* lexer);
void lexerSkipWhitespace(lexer_T* lexer);
char lexerPeek(lexer_T* lexer, int offset);

token_T* lexerParseId(lexer_T* lexer);
token_T* lexerParseNumber(lexer_T* lexer);

token_T* lexerAdvanceWith(lexer_T* lexer, token_T* token);
token_T* lexerAdvanceCurrent(lexer_T* lexer, tokenType_T type);

token_T* lexerNextToken(lexer_T* lexer);

#endif