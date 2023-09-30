#ifndef CSPYDR_INTERPRETER_H
#define CSPYDR_INTERPRETER_H

#include "ast/ast.h"
#include "config.h"
typedef struct INTERPRETER_CONTEXT_STRUCT
{

} InterpreterContext_T;

i32 interpreter_pass(Context_T* context, ASTProg_T* ast);

#endif
