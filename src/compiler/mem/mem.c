#include "mem.h"
#include "../config.h"
#include "../io/log.h"

#include <c-vector/vec.h>

void** allocs = NULL;
void** lists = NULL;

void mem_free(void)
{
    if(allocs)
    {
        for(size_t i = 0; i < vector_size(allocs); i++)
            free(allocs[i]);

        vector_free(allocs);
        allocs = NULL;
    }
    if(lists)
    {
        for(size_t i = 0; i < vector_size(lists); i++)
            free_list(lists[i]);

        vector_free(lists);
        lists = NULL;
    }
}

void* mem_malloc(size_t size)
{
    if(!allocs)
        allocs = vector_create();

    static int mallocs_failed = 0;
    void* ptr;
retry_malloc:
    ptr = malloc(size);
    if(!ptr)
    {
        if(mallocs_failed++ < MALLOC_RETRY_COUNT)
            goto retry_malloc;
        LOG_ERROR_F(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " allocating %ld bytes of memory\n", size);
        exit(1);
    }

    vector_add(&allocs, ptr);

    return ptr;
}

void mem_add_ptr(void* ptr)
{
    if(!allocs)
        allocs = vector_create();

    vector_add(&allocs, ptr);
}

void mem_add_list(List_T* list)
{
    if(!lists)
        lists = vector_create();

    vector_add(&lists, list);
}