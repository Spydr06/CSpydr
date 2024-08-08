#ifndef CSPYDR_AST_VTABLE_H
#define CSPYDR_AST_VTABLE_H

#include "ast.h"
#include "context.h"
#include "hashmap.h"

struct AST_VTABLE_STRUCT
{
    ASTType_T* interface;
    HashMap_T* functions;
    size_t id;
};

struct AST_IMPL_STRUCT
{
    ASTType_T* base_type;
    List_T* vtables;
};

ASTVTable_T* ast_vtable_init(Context_T* context, ASTType_T* interface);
ASTImpl_T* ast_impl_init(Context_T* context, ASTType_T* impl_base);

void vtable_register(Context_T* context, ASTProg_T* ast, ASTType_T* interface, ASTType_T* impl_base, ASTObj_T* function);
ASTVTable_T* vtable_get(Context_T* context, ASTProg_T* ast, ASTType_T* interface, ASTType_T* impl_base, bool create);

ASTObj_T* vtable_entry(ASTVTable_T* vtable, const char* ident);
ASTObj_T* vtable_insert(ASTVTable_T* vtable, ASTObj_T* function);

#endif

