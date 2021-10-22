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

void free_list(List_T* list)
{
    vector_free(list->items);
    free(list);
}

void list_insert(List_T* list, size_t pos, void* item)
{
    vector_insert(&list->items, pos, item);
}

void list_remove(List_T* list, size_t pos)
{
    vector_remove(list->items, pos);
}