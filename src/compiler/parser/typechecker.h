#ifndef CSPYDR_TYPECHECKER_H
#define CSPYDR_TYPECHECKER_H

#include <stdbool.h>
#include "ast/ast.h"
#include "validator.h"

void run_typechecker(ASTProg_T* ast);

bool types_equal(ASTType_T* a, ASTType_T* b);

bool implicitly_castable(Token_T* tok, ASTType_T* from, ASTType_T* to);
ASTNode_T* implicit_cast(Token_T* tok, ASTNode_T* expr, ASTType_T* to);

#endif