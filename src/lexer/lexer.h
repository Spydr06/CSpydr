#ifndef CSPYDR_LEXER_H
#define CSPYDR_LEXER_H

#include "../list.h"
#include "token.h"
#include "../io/file.h"

typedef struct LEXER_STRUCT {
    srcFile_T* file;

    char c;
    unsigned int line;
    unsigned int pos;
} lexer_T;

lexer_T* initLexer(srcFile_T* src);
void freeLexer(lexer_T* lexer);

token_T* lexerConsume(lexer_T* lexer, token_T* token);
token_T* lexerConsumeType(lexer_T* lexer, tokenType_T type);

token_T* lexerNextToken(lexer_T* lexer);

#endif