#include "list.h"

list_T* initList(size_t itemSize)
{
    list_T* list = calloc(1, sizeof(struct LIST_STRUCT));
    list->size = 0;
    list->itemSize = itemSize;
    list->items = 0;

    return list;
}

void listPush(list_T* list, void* item)
{
    list->size++;

    if(!list->items) {
        list->items = calloc(1, list->itemSize);
    }
    else {
        list->items = realloc(list->items, (list->size * list->itemSize));
    }

    list->items[list->size - 1] = item;
}
