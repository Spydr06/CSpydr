#ifndef CSPYDR_AST_MEM_H
#define CSPYDR_AST_MEM_H

#include "../../lexer/token.h"
#include <stdlib.h>

void ast_free(void);
void* ast_malloc(size_t size);
void ast_mem_add_ptr(void* ptr);
void ast_mem_add_list(List_T* list);

#endif