#include "stack.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "io/log.h"
#include "util.h"

#define STACK_HASSPACE(stack, additional) (((stack)->size + (additional) < (stack)->allocated))

InterpreterStack_T* init_interpreter_stack(size_t capacity)
{
    InterpreterStack_T* stack = malloc(capacity + sizeof(InterpreterStack_T));
    stack->size = 1;
    stack->allocated = MAX(capacity, 1);
    return stack;
}

size_t interpreter_stack_push(InterpreterStack_T** stack, const void* data, size_t size)
{
    if(!STACK_HASSPACE(*stack, size))
    {
        (*stack)->allocated = ((*stack)->size + size) * 2;
        *stack = realloc(*stack, (*stack)->allocated + sizeof(InterpreterStack_T)); // TODO: find better allocation curve
    }

    size_t start_addr = (*stack)->size;
    (*stack)->size += size;
    memcpy(&(*stack)->data[start_addr], data, size);

    return start_addr;
}

size_t interpreter_stack_align_to(InterpreterStack_T** stack, size_t align)
{
    return interpreter_stack_grow(stack, align_to((*stack)->size, align) - ((*stack)->size));
}

size_t interpreter_stack_grow(InterpreterStack_T** stack, size_t size)
{
    if(!STACK_HASSPACE(*stack, size))
    {
        (*stack)->allocated = ((*stack)->size + size) * 2;
        *stack = realloc(*stack, (*stack)->allocated + sizeof(InterpreterStack_T)); // TODO: find better allocation curve
    }

    return (*stack)->size += size;
}

void interpreter_stack_shrink_to(InterpreterStack_T* stack, size_t to) {
    assert(stack->size >= to);

    stack->size = to;
}

void free_interpreter_stack(InterpreterStack_T* stack)
{
    free(stack);
}

#define NUM_COLS 16

static inline void dump_line(const u8* data, size_t len)
{
    for(size_t i = 0; i < len / 2; i++)
        printf("%02x ", data[i]);
    printf("  ");
    for(size_t i = len / 2; i < len; i++)
        printf("%02x ", data[i]);
    printf(COLOR_RESET "| \"");
    for(size_t i = 0; i < len; i++)
        LOG_INFO_F("%c", isprint(data[i]) ? data[i] : '.');
    printf("\"\n");
}

void dump_stack(InterpreterStack_T* stack)
{
    for(size_t i = 0; i < stack->size / NUM_COLS + 1; i++)
    {
        printf(COLOR_BOLD_MAGENTA "%06lx " COLOR_RESET COLOR_YELLOW, i * NUM_COLS);

        if(stack->size - i * NUM_COLS < NUM_COLS)
        {
            u8 buf[NUM_COLS] = {};
            memset(buf, 0, sizeof buf);
            memcpy(buf, stack->data + i * NUM_COLS, stack->size - i * NUM_COLS);
            dump_line(buf, NUM_COLS);
        }
        else
            dump_line(stack->data + i * NUM_COLS, NUM_COLS);
    }
}
