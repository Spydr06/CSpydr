#ifndef CSPYDR_C_PARSER_H
#define CSPYDR_C_PARSER_H

#include "ast/ast.h"
#include "config.h"
#include "context.h"
#include "lexer/token.h"
#include "list.h"

typedef struct C_PARSER_STRUCT {
    Context_T* context;
    ASTProg_T* ast;
    ASTObj_T* current_obj;

    List_T* include_dirs;
} CParser_T;

void c_parser_init(CParser_T* parser, Context_T* context, ASTProg_T* ast);
void c_parser_free(CParser_T* parser);

void parse_c_header(CParser_T* parser, ASTObj_T* surrounding_obj, Token_T* include_token, const char* header);

void c_parser_add_include_dir(CParser_T* parser, const char* dir);

#endif

