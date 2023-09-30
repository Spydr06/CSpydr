#ifndef CSPYDR_INTERPRETER_H
#define CSPYDR_INTERPRETER_H

#include "ast/ast.h"
#include "config.h"
#include "stack.h"

typedef struct INTERPRETER_CONTEXT_STRUCT
{
    InterpreterStack_T* stack;
    Context_T* context;
    ASTProg_T* ast;
} InterpreterContext_T;

i32 interpreter_pass(Context_T* context, ASTProg_T* ast);

#endif
