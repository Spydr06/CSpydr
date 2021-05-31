#ifndef CSPYDR_PARSER_H
#define CSPYDR_PARSER_H

#include "../lexer/lexer.h"
#include "../error/errorHandler.h"
#include "../ast/ast.h"

typedef struct PARSER_STRUCT Parser_T;

typedef ASTExpr_T* (*prefix_parse_fn)(Parser_T* parser);
typedef ASTExpr_T* (*infix_parse_fn)(Parser_T* parser, ASTExpr_T* left);

struct PARSER_STRUCT
{
    Lexer_T* lexer;
    ErrorHandler_T* eh;
    ASTProgram_T* root_ref;
    Token_T* tok;
    List_T* imports;

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

Parser_T* init_parser(Lexer_T* lexer);
void free_parser(Parser_T* parser);

Token_T* parser_advance(Parser_T* parser);
Token_T* parser_consume(Parser_T* parser, TokenType_T type, const char* msg);

ASTProgram_T* parse(Parser_T* parser, const char* mainFile);

#endif