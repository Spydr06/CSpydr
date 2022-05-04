#ifndef CSPYDR_HASHMAP_H

#include <stdlib.h>

typedef struct HASHMAP_STRUCT HashMap_T;

HashMap_T* hashmap_init();
HashMap_T* hashmap_init_sized(size_t size);
void hashmap_free(HashMap_T* map);
int hashmap_put(HashMap_T* map, char* key, void* val);
void* hashmap_get(HashMap_T* map, char* key);

void hashmap_set_hash_function(size_t (*function)(char* data));

#endif