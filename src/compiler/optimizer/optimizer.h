#ifndef CSPYDR_OPTIMIZER_H
#define CSPYDR_OPTIMIZER_H

#include "ast/ast.h"

i32 optimizer_pass(Context_T* context, ASTProg_T* ast);

#endif