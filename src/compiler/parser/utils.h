#ifndef CSPYDR_PARSER_UTILS_H
#define CSPYDR_PARSER_UTILS_H

#include "ast/ast.h"

#define either(a, b) ((a) ? (a) : (b))

extern ASTObj_T alloca_bottom;
void collect_locals(ASTNode_T* stmt, List_T* locals);
bool identifiers_equal(ASTIdentifier_T* a, ASTIdentifier_T* b);
ASTNode_T* unpack_closure_and_casts(ASTNode_T* node);

#endif