#include "mem.h"

#include "config.h"
#include "io/log.h"
#include "list.h"

static List_T* allocs = NULL;
static List_T* lists = NULL;

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

    return ptr;
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