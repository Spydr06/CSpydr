#ifndef CSPYDR_ACT_PARSER_H
#define CSPYDR_ACT_PARSER_H

#include "AST.h"
#include "../lexer.h"
#include "../token.h"

typedef struct PARSER_STRUCT
{
    lexer_T* lexer;
    token_T* token;
    ASTRoot_T* rootRef;
    list_T* localVars;
} parser_T;

parser_T* initParser(lexer_T* lexer);

token_T* parserAdvance(parser_T* parser);
token_T* parserConsume(parser_T* parser, tokenType_T type, const char* msg);

ASTRoot_T* parserParse(parser_T* parser);
ASTCompound_T* parserParseCompound(parser_T* parser);
ASTDataType_T* parserParseDataType(parser_T* parser);
ASTLocal_T* parserParseLocal(parser_T* parser, bool includeLet);

#endif