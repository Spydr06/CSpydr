#include "list.h"

#include <c-vector/vec.h>

List_T* init_list(size_t item_size)
{
    List_T* list = malloc(sizeof(struct LIST_STRUCT));
    list->size = 0;
    list->item_size = 0;
    list->items = vector_create();

    return list;
}

void list_push(List_T* list, void* item)
{
    list->size++;
    vector_add(&list->items, item);
}

void* list_pop(List_T* list)
{
    if(list->size > 0)
    {
        void* top = list->items[--list->size];
        vector_pop(list->items);
        return top;
    }
    else
        return NULL;
}

void free_list(List_T* list)
{
    vector_free(list->items);
    free(list);
}

void list_insert(List_T* list, size_t pos, void* item)
{
    list->size++;
    vector_insert(&list->items, pos, item);
}

void list_remove(List_T* list, size_t pos)
{
    list->size--;
    vector_remove(list->items, pos);
}

void list_clear(List_T* list)
{
    list->size = 0;
    vector_erase(list->items, 0, vector_size(list->items));
}