#ifndef CSPYDR_INTERPRETER_H
#define CSPYDR_INTERPRETER_H

#include "ast/ast.h"
#include "config.h"
#include "hashmap.h"
#include "stack.h"

#include "value.h"

typedef struct INTERPRETER_CONTEXT_STRUCT
{
    Context_T* context;
    ASTProg_T* ast;
    bool constexpr_only;

    InterpreterStack_T* global_storage;
    InterpreterStack_T* stack;
    size_t recursion_depth;
    
    InterpreterValue_T pipe_value;
    HashMap_T* string_literals;

    bool broken;    // reached `break`
    bool continued; // reached `continue`
    bool returned;  // reached `return`
    InterpreterValue_T return_value;
} InterpreterContext_T;

void init_interpreter_context(InterpreterContext_T* ictx, Context_T* context, ASTProg_T* ast);
void free_interpreter_context(InterpreterContext_T* ictx);

i32 interpreter_pass(Context_T* context, ASTProg_T* ast);

InterpreterValue_T interpreter_eval_expr(InterpreterContext_T* ictx, ASTNode_T* expr);

#endif
