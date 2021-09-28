#ifndef CSPYDR_AST_ITERATOR_H
#define CSPYDR_AST_ITERATOR_H

#include "ast.h"
#include <stdarg.h>

typedef void (*ASTNodeIteratorFn_T)(ASTNode_T* node, va_list custom_args);
typedef void (*ASTTypeIteratorFn_T)(ASTType_T* type, va_list custom_args);
typedef void (*ASTObjIteratorFn_T)(ASTObj_T* obj, va_list custom_args);
typedef void (*ASTIdIteratorFn_T)(ASTIdentifier_T* id, va_list custom_args);

typedef struct AST_ITERATOR_LIST_STRUCT
{
    ASTNodeIteratorFn_T node_fns[ND_KIND_LEN];
    ASTTypeIteratorFn_T type_fns[TY_KIND_LEN];

    ASTObjIteratorFn_T obj_start_fns[OBJ_KIND_LEN];
    ASTObjIteratorFn_T obj_end_fns[OBJ_KIND_LEN];
    
    ASTIdIteratorFn_T id_def_fn;
    ASTIdIteratorFn_T id_use_fn;
} ASTIteratorList_T;

void ast_iterate(ASTIteratorList_T* list, ASTProg_T* ast, ...);

#endif