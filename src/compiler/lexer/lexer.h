#ifndef CSPYDR_LEXER_H
#define CSPYDR_LEXER_H

#include "../list.h"
#include "token.h"
#include "../io/file.h"
#include "../error/error.h"

typedef struct LEXER_STRUCT {
    SrcFile_T* file;

    char c;
    unsigned int line;
    unsigned int pos;
} Lexer_T;

Lexer_T* init_lexer(SrcFile_T* src);
void     free_lexer(Lexer_T* lexer);
Token_T* lexer_consume(Lexer_T* lexer, Token_T* token);
Token_T* lexer_consume_type(Lexer_T* lexer, TokenType_T type);
Token_T* lexer_next_token(Lexer_T* lexer);

#endif