#ifndef CSPYDR_INTERPRETER_H
#define CSPYDR_INTERPRETER_H

#include "ast/ast.h"
#include "config.h"
#include "hashmap.h"
#include "stack.h"

#include "value.h"

typedef struct INTERPRETER_CONTEXT_STRUCT
{
    InterpreterStack_T* stack;
    Context_T* context;
    ASTProg_T* ast;

    InterpreterValue_T pipe_value;
    HashMap_T* string_literals;

    bool broken;    // reached `break`
    bool continued; // reached `continue`
    bool returned;  // reached `return`
    InterpreterValue_T return_value;
} InterpreterContext_T;

i32 interpreter_pass(Context_T* context, ASTProg_T* ast);

#endif
