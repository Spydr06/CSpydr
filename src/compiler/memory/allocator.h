#ifndef CSPYDR_MEMORY_ALLOCATOR_H
#define CSPYDR_MEMORY_ALLOCATOR_H

#include <stdlib.h>
#include "list.h"

typedef struct CSPYDR_ALLOCATOR_STRUCT {
    List_T* pointers;
    void (*free_func)(void*);
} Allocator_T;

void init_allocator(Allocator_T* alloc, void (*free_func)(void*));
void free_allocator(Allocator_T* alloc);

void* allocator_malloc(Allocator_T* alloc, size_t malloc_size);
void* allocator_realloc(Allocator_T* alloc, void* ptr, size_t new_size);
void* allocator_push(Allocator_T* alloc, void* elem);

#endif

