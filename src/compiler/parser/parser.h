#ifndef CSPYDR_PARSER_H
#define CSPYDR_PARSER_H

#include "../lexer/lexer.h"
#include "../error/errorHandler.h"
#include "../ast/ast.h"

typedef struct PARSER_STRUCT parser_T;

typedef ASTExpr_T* (*prefixParseFn)(parser_T* parser);
typedef ASTExpr_T* (*infixParseFn)(parser_T* parser, ASTExpr_T* left);

struct PARSER_STRUCT
{
    lexer_T* lexer;
    errorHandler_T* eh;
    ASTProgram_T* rootRef;
    list_T* localVars;
    token_T* tok;
    list_T* imports;

    bool silent;
};

typedef enum {
    LOWEST  =  0,
    ASSIGN  =  1, // x = y, x += y
    EQUALS  =  2, // ==
    LTGT    =  3, // < >
    SUM     =  4, // + -
    PRODUCT =  5, // * /
    POSTFIX =  6, // x++, x--
    PREFIX  =  7, // -x, !x
    CALL    =  8, // x(y)
    INDEX   =  9, // x[y]
    HIGHEST = 10,
} precedence_T;

parser_T* initParser(lexer_T* lexer);
void freeParser(parser_T* parser);

token_T* parserAdvance(parser_T* parser);
token_T* parserConsume(parser_T* parser, tokenType_T type, const char* msg);

ASTProgram_T* parserParse(parser_T* parser, const char* mainFile);

#endif