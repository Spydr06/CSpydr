#ifndef CSPYDR_LEXER_H
#define CSPYDR_LEXER_H

#include "ast/ast.h"
#include "config.h"
#include "list.h"
#include "token.h"
#include "io/file.h"
#include "error/error.h"

typedef struct LEXER_STRUCT 
{
    Context_T* context;
    File_T* file;

    char c;          // current character
    size_t line;     // current line
    size_t pos;      // current position
} Lexer_T;

void init_lexer(Lexer_T* lexer, Context_T* context, File_T* src);
Token_T* lexer_consume(Lexer_T* lexer, Token_T* token);
Token_T* lexer_consume_type(Lexer_T* lexer, TokenType_T type);
Token_T* lexer_next_token(Lexer_T* lexer);
bool token_is_keyword(TokenType_T type);

i32 lexer_pass(Context_T* context, ASTProg_T* ast);

#endif