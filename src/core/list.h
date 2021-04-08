#ifndef CSPYDR_LIST_H
#define CSPYDR_LIST_H

#include <stdlib.h>

typedef struct LIST_STRUCT
{
    void** items;
    size_t size;
    size_t itemSize;
} list_T;

list_T* initList(size_t itemsize);
void freeList(list_T* list);

void listPush(list_T* list, void* item);

#endif