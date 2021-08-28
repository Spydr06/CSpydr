#include "ast_mem.h"

#include <c-vector/vec.h>

void** allocs = NULL;
void** lists = NULL;

void ast_free(void)
{
    if(allocs)
    {
        for(size_t i = 0; i < vector_size(allocs); i++)
            free(allocs[i]);

        vector_free(allocs);
    }
    if(lists)
    {
        for(size_t i = 0; i < vector_size(lists); i++)
            free_list(lists[i]);

        vector_free(lists);
    }
}

void* ast_malloc(size_t size)
{
    if(!allocs)
        allocs = vector_create();

    void* ptr = malloc(size);

    vector_add(&allocs, ptr);

    return ptr;
}

void ast_mem_add_ptr(void* ptr)
{
    if(!allocs)
        allocs = vector_create();

    vector_add(&allocs, ptr);
}

void ast_mem_add_list(List_T* list)
{
    if(!lists)
        lists = vector_create();

    vector_add(&lists, list);
}