#ifndef CSPYDR_VALIDATOR_H
#define CSPYDR_VALIDATOR_H

#include "../ast/ast.h"
#include "../list.h"

typedef struct VALIDATOR_SCOPE_STRUCT VScope_T;

typedef struct VALIDATOR_STRUCT Validator_T;

void validate_ast(ASTProg_T* ast);

#endif