#ifndef CSPYDR_HASHMAP_H

#include <stdlib.h>
#include "list.h"

typedef struct HASHMAP_STRUCT HashMap_T;

HashMap_T* hashmap_init();
HashMap_T* hashmap_init_sized(size_t size);
void hashmap_free(HashMap_T* map);
int hashmap_put(HashMap_T* map, char* key, void* val);
void* hashmap_get(HashMap_T* map, char* key);
List_T* hashmap_values(HashMap_T* map);
List_T* hashmap_keys(HashMap_T* map);

size_t hashmap_default_hash(char* data);
void hashmap_set_hash_function(size_t (*function)(char* data));

#endif