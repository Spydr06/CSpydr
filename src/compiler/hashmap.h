#ifndef CSPYDR_HASHMAP_H

typedef struct HASHMAP_STRUCT HashMap_T;

HashMap_T* hashmap_init();
void hashmap_free(HashMap_T* map);
int hashmap_put(HashMap_T* map, char* key, void* val);
void* hashmap_get(HashMap_T* map, char* key);

#endif