#ifndef CSPYDR_MEM_H
#define CSPYDR_MEM_H

#include "lexer/token.h"
#include <stdlib.h>

void mem_free(void);
void* mem_malloc(size_t size);
void mem_add_ptr(void* ptr);
void mem_add_list(List_T* list);

#endif