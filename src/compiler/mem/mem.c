#include "mem.h"

#include "config.h"
#include "io/log.h"
#include "list.h"
#include "hashmap.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static List_T* allocs = NULL;
static List_T* lists = NULL;
static List_T* hashmaps = NULL;

void mem_free(void)
{
    if(allocs)
    {
        for(size_t i = 0; i < allocs->size; i++)
            free(allocs->items[i]);

        free_list(allocs);
        allocs = NULL;
    }
    if(lists)
    {
        for(size_t i = 0; i < lists->size; i++)
            free_list(lists->items[i]);

        free_list(lists);
        lists = NULL;
    }
    if(hashmaps)
    {
        for(size_t i = 0; i < hashmaps->size; i++)
            hashmap_free(hashmaps->items[i]);
        
        free_list(hashmaps);
        hashmaps = NULL;
    }
}

void* mem_malloc(size_t size)
{
    if(!allocs)
        allocs = init_list();

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

    list_push(allocs, ptr);
    memset(ptr, 0, size);

    return ptr;
}

void* mem_realloc(void* ptr, size_t size)
{
    if(!allocs)
        allocs = init_list();
    
    for(size_t i = 0; i < allocs->size; i++) {
        if(allocs->items[i] == ptr) {
            allocs->items[i] = realloc(allocs->items[i], size);
            return allocs->items[i];
        }
    }

    return mem_malloc(size);
}

void mem_add_ptr(void* ptr)
{
    if(!allocs)
        allocs = init_list();

    list_push(allocs, ptr);
}

void mem_add_list(List_T* list)
{
    if(!lists)
        lists = init_list();

    list_push(lists, list);
}

void mem_add_hashmap(HashMap_T* map)
{
    if(!hashmaps)
        hashmaps = init_list();
    
    list_push(hashmaps, map);
}