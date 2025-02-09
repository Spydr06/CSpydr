#include "allocator.h"
#include "config.h"
#include "io/log.h"

#include <stdint.h>

#define LAZY_INIT(list) do {   \
    if(!(list))                \
        (list) = init_list();  \
    } while(0)

void init_allocator(Allocator_T* alloc, void (*free_func)(void*))
{
    alloc->pointers = NULL; // lazy initialization
    alloc->free_func = free_func;
}

void free_allocator(Allocator_T* alloc)
{
    if(!alloc->pointers) 
        return;
    
    for(size_t i = 0; i < alloc->pointers->size; i++)
        alloc->free_func(alloc->pointers->items[i]);
    free_list(alloc->pointers);
    alloc->pointers = NULL;
}

void* allocator_malloc(Allocator_T* alloc, size_t malloc_size)
{
    LAZY_INIT(alloc->pointers);

    uint32_t retry = 0;
    void* ptr;
    do {
        ptr = calloc(1, malloc_size);
        if(retry++ > MALLOC_RETRY_COUNT)
        {
            LOG_ERROR_F(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " allocating %ld bytes of memory\n", malloc_size);
            exit(2);
        }
    } while(!ptr);

    list_push(alloc->pointers, ptr);
    return ptr;
}

void* allocator_realloc(Allocator_T* alloc, void* ptr, size_t new_size)
{
    LAZY_INIT(alloc->pointers);

    for(size_t i = 0; i < alloc->pointers->size; i++) {
        if(alloc->pointers->items[i] == ptr) {
            alloc->pointers->items[i] = realloc(alloc->pointers->items[i], new_size);
            return alloc->pointers->items[i];
        }
    }

    return allocator_malloc(alloc, new_size);
}

void* allocator_push(Allocator_T* alloc, void* elem)
{
    LAZY_INIT(alloc->pointers);
    list_push(alloc->pointers, elem);
    return elem;
}


