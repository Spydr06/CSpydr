#ifndef CSPYDR_CONSTEXPR_H
#define CSPYDR_CONSTEXPR_H

#include "../util.h"
#include "../ast/ast.h"

u64 const_i64(ASTNode_T* node);
void evaluate_const_exprs(ASTProg_T* ast);

#endif