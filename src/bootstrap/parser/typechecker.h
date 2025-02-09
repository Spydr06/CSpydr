#ifndef CSPYDR_TYPECHECKER_H
#define CSPYDR_TYPECHECKER_H

#include <stdbool.h>
#include "ast/ast.h"
#include "config.h"
#include "list.h"

typedef enum IMPLICIT_CAST_RESULT {
    CAST_OK             = 1,
    CAST_DYN            = 2,
    CAST_UNDYN          = 4,
    CAST_ERR            = 8,
    CAST_DELETING_CONST = 16
} ImplicitCastResult_T;

typedef struct TYPECHECKER_STRUCT {
    Context_T* context;
    ASTProg_T* ast;
    ASTObj_T* current_fn;
    List_T* return_type_stack;
} TypeChecker_T;

void typechecker_init(TypeChecker_T* t, Context_T* context, ASTProg_T* ast);
void typechecker_free(TypeChecker_T* t);

void typecheck_obj(TypeChecker_T* t, ASTObj_T* obj);

bool types_equal(Context_T* context, ASTType_T* a, ASTType_T* b);

ImplicitCastResult_T implicitly_castable(TypeChecker_T* t, Token_T* tok, ASTType_T* from, ASTType_T* to);
ASTNode_T* implicit_cast(TypeChecker_T* t, Token_T* tok, ASTNode_T* expr, ASTType_T* to, ImplicitCastResult_T result);

#endif
