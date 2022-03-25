#ifndef CSPYDR_PARSER_UTILS_H
#define CSPYDR_PARSER_UTILS_H

#include "ast/ast.h"

extern ASTObj_T alloca_bottom;
void collect_locals(ASTNode_T* stmt, List_T* locals);

#endif