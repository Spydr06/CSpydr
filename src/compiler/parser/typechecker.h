#ifndef CSPYDR_TYPECHECKER_H
#define CSPYDR_TYPECHECKER_H

#include <stdbool.h>
#include "ast/ast.h"
#include "validator.h"

bool types_equal(ASTType_T* a, ASTType_T* b);

void typecheck_assignment(Validator_T* v, ASTNode_T* assignment);
ASTNode_T* typecheck_arg_pass(Validator_T* v, ASTType_T* expected, ASTNode_T* received);

bool implicitly_castable(Validator_T* v, Token_T* tok, ASTType_T* from, ASTType_T* to);
ASTNode_T* implicit_cast(Token_T* tok, ASTNode_T* expr, ASTType_T* to);

#endif