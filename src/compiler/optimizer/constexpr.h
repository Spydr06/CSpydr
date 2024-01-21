#ifndef CSPYDR_CONSTEXPR_H
#define CSPYDR_CONSTEXPR_H

#include "config.h"
#include "interpreter/interpreter.h"
#include "util.h"
#include "ast/ast.h"

typedef InterpreterContext_T ConstexprResolver_T;

void init_constexpr_resolver(ConstexprResolver_T* resolver, Context_T* context, ASTProg_T* ast);
void free_constexpr_resolver(ConstexprResolver_T* resolver);

ASTNode_T* eval_constexpr(ConstexprResolver_T* resolver, ASTNode_T* expr);

u64 const_u64(Context_T* context, ASTNode_T* node);
i64 const_i64(Context_T* context, ASTNode_T* node);
void evaluate_const_exprs(ASTProg_T* ast);

#endif
