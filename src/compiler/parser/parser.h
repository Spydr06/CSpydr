#ifndef CSPYDR_PARSER_H
#define CSPYDR_PARSER_H

#include "../lexer/lexer.h"
#include "../error/error.h"
#include "../ast/ast.h"
#include "../lexer/preprocessor.h"

typedef struct PARSER_STRUCT Parser_T;

typedef ASTNode_T* (*prefix_parse_fn)(Parser_T* parser);
typedef ASTNode_T* (*infix_parse_fn)(Parser_T* parser, ASTNode_T* left);

Parser_T* init_parser(List_T* tokens);
void free_parser(Parser_T* parser);

ASTProg_T* parse(List_T* files, bool is_silent);

#endif