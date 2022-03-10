#ifndef CSPYDR_PARSER_H
#define CSPYDR_PARSER_H

#include "../ast/ast.h"

void parse(ASTProg_T* ast, List_T* files, bool is_silent);
ASTNode_T* build_str_lit(Token_T* tok, char* str, bool allocate_global, List_T* objs);

#endif