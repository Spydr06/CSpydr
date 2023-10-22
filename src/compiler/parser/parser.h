#ifndef CSPYDR_PARSER_H
#define CSPYDR_PARSER_H

#include "ast/ast.h"

typedef struct PARSER_STRUCT Parser_T;

i32 parser_pass(Context_T* context, ASTProg_T* ast);

Context_T* parser_context(Parser_T* p);
ASTProg_T* parser_ast(Parser_T* p);
Token_T* parser_consume(Parser_T* p, TokenType_T type, const char* msg);
Token_T* parser_peek(Parser_T* p, i32 level);

bool tok_is(Parser_T* p, TokenType_T type);

void parse_obj(Parser_T* p, List_T* obj_list);

#endif
