#ifndef CSPYDR_IR_MANGLE_H
#define CSPYDR_IR_MANGLE_H

#include "ast/ast.h"
#include "context.h"

char* mangle_identifier(Context_T* context, ASTIdentifier_T* ident);

#endif

