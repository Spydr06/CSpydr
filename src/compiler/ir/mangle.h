#ifndef CSPYDR_IR_MANGLE_H
#define CSPYDR_IR_MANGLE_H

#include "ast/ast.h"

#include "list.h"
#include "normalizer.h"

#define MANGLE_STATIC_PREFIX "__csp$"

typedef struct MANGLED_IDENTIFIER_STRUCT 
{
    ASTObj_T* backing;
    const char* mangled_ident;
} MangledIdentifier_T;

const char* mangle_identifier(Normalizer_T* n, ASTIdentifier_T* ident, ASTObj_T* backing);

void free_mangled_idents(List_T** mangled_idents);

#endif

