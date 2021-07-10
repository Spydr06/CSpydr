#ifndef CSPYDR_PARSER_H
#define CSPYDR_PARSER_H

#include "../lexer/lexer.h"
#include "../error/error.h"
#include "../ast/ast.h"
#include "../lexer/preprocess.h"

typedef struct PARSER_STRUCT Parser_T;

typedef ASTNode_T* (*prefix_parse_fn)(Parser_T* parser);
typedef ASTNode_T* (*infix_parse_fn)(Parser_T* parser, ASTNode_T* left);

struct PARSER_STRUCT
{
    List_T* tokens;
    size_t token_i;
    ASTProg_T* root_ref;
    Token_T* tok;
    ASTNode_T* cur_block;
    ASTObj_T* cur_fn;

    size_t cur_lambda_id;
    size_t cur_tuple_id;
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
    MEMBER  = 10, // x.y
    CLOSURE = 11, // (x + y) * z
    CAST    = 12, // x:i32
    HIGHEST = 13,
} Precedence_T;

Parser_T* init_parser(List_T* tokens);
void free_parser(Parser_T* parser);

ASTProg_T* parse(List_T* files, bool is_silent);

#endif