#ifndef CSPYDR_LIST_H
#define CSPYDR_LIST_H

#include <stdlib.h>

typedef struct LIST_STRUCT
{
    void** items;
    size_t size;
    size_t item_size;
} List_T;

List_T* init_list(size_t itemsize);
void free_list(List_T* list);

void list_push(List_T* list, void* item);
void list_insert(List_T* list, size_t pos, void* item);
void list_remove(List_T* list, size_t pos);

#endif