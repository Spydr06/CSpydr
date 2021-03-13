#ifndef CSPYDR_PARSER_H
#define CSPYDR_PARSER_H

#include "lexer.h"
#include "AST.h"

typedef struct PARSER_STRUCT
{
    lexer_T* lexer;
    token_T* token;
} parser_T;

parser_T* initParser(lexer_T* lexer);

token_T* parserEat(parser_T* parser, int type);

AST_T* parserParse(parser_T* parser);
AST_T* parserParseCompound(parser_T* parser);
AST_T* parserParseNumber(parser_T* parser);
AST_T* parserParseExpr(parser_T* parser);
AST_T* parserParseId(parser_T* parser);
AST_T* parserParseList(parser_T* parser);
AST_T* parserParseBlock(parser_T* parser);
AST_T* parserParseStmt(parser_T* parser);
AST_T* parserParseVarDeclaration(parser_T* parser);

#endif