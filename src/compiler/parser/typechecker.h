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

typedef struct TYPECHECKER_STRUCT {
    Context_T* context;
    ASTObj_T* current_fn;
    List_T* return_type_stack;
} TypeChecker_T;

void typechecker_init(TypeChecker_T* t, Context_T* context);
void typechecker_free(TypeChecker_T* t);

void typecheck_obj(TypeChecker_T* t, ASTObj_T* obj);

bool types_equal(Context_T* context, ASTType_T* a, ASTType_T* b);

enum IMPLICIT_CAST_RESULT implicitly_castable(Context_T* contest, Token_T* tok, ASTType_T* from, ASTType_T* to);
ASTNode_T* implicit_cast(Context_T* context, Token_T* tok, ASTNode_T* expr, ASTType_T* to);

#endif
