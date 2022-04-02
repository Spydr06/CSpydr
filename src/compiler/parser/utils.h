#ifndef CSPYDR_PARSER_UTILS_H
#define CSPYDR_PARSER_UTILS_H

#include "ast/ast.h"

extern ASTObj_T alloca_bottom;
void collect_locals(ASTNode_T* stmt, List_T* locals);
bool identifiers_equal(ASTIdentifier_T* a, ASTIdentifier_T* b);
ASTNode_T* build_str_lit(Token_T* tok, char* str, bool allocate_global, List_T* objs);

#endif