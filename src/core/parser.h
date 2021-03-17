#ifndef CSPYDR_PARSER_H
#define CSPYDR_PARSER_H

#include "AST.h"
#include "lexer.h"
#include "token.h"

typedef struct PARSER_STRUCT
{
    lexer_T* lexer;
    token_T* token;
} parser_T;

parser_T* initParser(lexer_T* lexer);

token_T* parserAdvance(parser_T* parser);
token_T* parserConsume(parser_T* parser, tokenType_T type, const char* msg);

AST_T* parserParse(parser_T* parser);

#endif