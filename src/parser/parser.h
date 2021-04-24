#ifndef CSPYDR_PARSER_H
#define CSPYDR_PARSER_H

#include "../lexer/lexer.h"
#include "../error/errorHandler.h"
#include "../ast/ast.h"

typedef ASTExpr_T* (*prefixParseFn)();
typedef ASTExpr_T* (*infixParseFn)(ASTExpr_T* left);

#define NUM_PREFIX_PARSE_FNS 0
#define NUM_INFIX_PARSE_FNS 0

typedef struct PARSER_STRUCT
{
    lexer_T* lexer;
    errorHandler_T* eh;
    ASTRoot_T* rootRef;
    list_T* localVars;
    token_T* tok;

    struct {tokenType_T tt; prefixParseFn fn;} prefixParseFns[NUM_PREFIX_PARSE_FNS];
    struct {tokenType_T tt; infixParseFn fn;}  infixParseFns [NUM_INFIX_PARSE_FNS];
} parser_T;

typedef enum {
    LOWEST  = 0,
    EQUALS  = 1, // ==
    LTGT    = 2, // < >
    SUM     = 3, // + -
    PRODUCT = 4, // * /
    PREFIX  = 5, // -x, !x
    CALL    = 6, // x(y)
    INDEX   = 7, // x[y]
    HIGHEST = 8,
} precedence_T;

parser_T* initParser(lexer_T* lexer);
void freeParser(parser_T* parser);

token_T* parserAdvance(parser_T* parser);
token_T* parserConsume(parser_T* parser, tokenType_T type, const char* msg);

ASTRoot_T* parserParse(parser_T* parser);

#endif