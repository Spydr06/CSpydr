#ifndef CSPYDR_INTERPRETER_STACK_H
#define CSPYDR_INTERPRETER_STACK_H

#include <stddef.h>

#include "util.h"

typedef struct INTERPRETER_STACK_STRUCT
{
    size_t size;
    size_t allocated;
    u8 data[];
} InterpreterStack_T;

InterpreterStack_T* init_interpreter_stack(size_t capacity);
void free_interpreter_stack(InterpreterStack_T* stack);

size_t interpreter_stack_push(InterpreterStack_T** stack, const void* data, size_t size);

size_t interpreter_stack_align_to(InterpreterStack_T** stack, size_t align);
size_t interpreter_stack_grow(InterpreterStack_T** stack, size_t size);
void interpreter_stack_shrink_to(InterpreterStack_T* stack, size_t to);

#define STACK_TOP(stack) ((stack)->size)

void dump_stack(InterpreterStack_T* stack);

#endif
