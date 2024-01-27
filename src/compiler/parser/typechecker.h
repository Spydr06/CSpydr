#ifndef CSPYDR_TYPECHECKER_H
#define CSPYDR_TYPECHECKER_H

#include <stdbool.h>
#include "ast/ast.h"
#include "config.h"

enum IMPLICIT_CAST_RESULT {
    CAST_OK,
    CAST_ERR,
    CAST_DELETING_CONST
};

i32 typechecker_pass(Context_T* context, ASTProg_T* ast);

bool types_equal(Context_T* context, ASTType_T* a, ASTType_T* b);

enum IMPLICIT_CAST_RESULT implicitly_castable(Context_T* contest, Token_T* tok, ASTType_T* from, ASTType_T* to);
ASTNode_T* implicit_cast(Context_T* context, Token_T* tok, ASTNode_T* expr, ASTType_T* to);

#endif
