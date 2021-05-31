#include "list.h"

List_T* init_list(size_t item_size)
{
    List_T* list = calloc(1, sizeof(struct LIST_STRUCT));
    list->size = 0;
    list->item_size = item_size;
    list->items = 0;

    return list;
}

void list_push(List_T* list, void* item)
{
    list->size++;

    if(!list->items) {
        list->items = calloc(1, list->item_size);
    }
    else {
        list->items = realloc(list->items, (list->size * list->item_size));
    }

    list->items[list->size - 1] = item;
}

void free_list(List_T* list)
{
    free(list->items);
    free(list);
}
