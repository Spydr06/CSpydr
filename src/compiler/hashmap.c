#include "hashmap.h"
#include "list.h"
#include "util.h"

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#ifndef HASHMAP_INIT_SIZE
    #define HASHMAP_INIT_SIZE 128
#endif

#define HASHMAP_SIZE_MOD(map, val)     ((val) & ((map)->alloc - 1))
#define HASHMAP_PROBE_NEXT(map, index) (HASHMAP_SIZE_MOD(map, (index) + 1))

typedef struct HASH_PAIR_STRUCT
{
    char* key;
    void* value;
} HashPair_T;

struct HASHMAP_STRUCT
{
    size_t size;
    size_t alloc;
    HashPair_T* pairs;
};

static inline size_t hashmap_calc_size(HashMap_T* map);
static inline void hashmap_rehash(HashMap_T* map, size_t size);
static inline HashPair_T* hashmap_find_pair(const HashMap_T* map, const char* key, bool find_empty);

static size_t (*HASHMAP_HASH_FUNCTION)(const char*) = hashmap_default_hash;

HashMap_T* hashmap_init()
{
    return hashmap_init_sized(HASHMAP_INIT_SIZE);
}

HashMap_T* hashmap_init_sized(size_t size)
{
    size_t actual_size = 1 << ((sizeof(u64) << 3) - __builtin_clzl(size - 1)); // round up to the next power of two

    HashMap_T* map = malloc(sizeof(struct HASHMAP_STRUCT));
    map->size = 0;
    map->alloc = actual_size;
    map->pairs = calloc(map->alloc, sizeof(HashPair_T));

    return map;
}

void hashmap_free(HashMap_T* map)
{
    free(map->pairs);
    free(map);
}

int hashmap_put(HashMap_T* map, char* key, void* value)
{
    if(!key || !map)
        return EINVAL;

    size_t map_size = hashmap_calc_size(map);
    
    if(map_size > map->size)
        hashmap_rehash(map, map_size);

    HashPair_T* pair = hashmap_find_pair(map, key, true);
    if(!pair)
        return EADDRNOTAVAIL; // out of memory
    
    if(pair->key) 
        // data already exists
        return EEXIST;

    pair->key = key;
    pair->value = value;
    map->size++;

    return 0;
}

void hashmap_set_hash_function(size_t (*function)(const char*))
{
    HASHMAP_HASH_FUNCTION = function;
}

void* hashmap_get(const HashMap_T* map, const char* key)
{
    HashPair_T* pair = hashmap_find_pair(map, key, false);
    return pair ? pair->value : NULL;
}

List_T* hashmap_values(HashMap_T* map)
{
    List_T* list = init_list_sized(map->size);
    for(HashPair_T* pair = map->pairs; pair < &map->pairs[map->alloc]; pair++)
    {
        if(pair->key)
            list_push(list, pair->value);
    }

    return list;
}

List_T* hashmap_keys(HashMap_T* map)
{
    List_T* list = init_list();
    for(HashPair_T* pair = map->pairs; pair < &map->pairs[map->alloc]; pair++)
    {
        if(pair->key)
            list_push(list, pair->key);
    }

    return list;
}

static inline size_t hashmap_calc_size(HashMap_T* map)
{
    size_t map_size = map->size + (map->size / 3);
    if(map_size < map->alloc)
        return map->alloc;
    else
        return 1 << ((sizeof(u64) << 3) - __builtin_clzl(map_size - 1));
}

static inline void hashmap_rehash(HashMap_T* map, size_t size)
{
    if(!map || size < map->size)
        return;

    size_t old_alloc = map->alloc;
    HashPair_T* old_pairs = map->pairs;
    map->alloc = size;
    map->pairs = calloc(size, sizeof(HashPair_T));

    // rehash
    for(HashPair_T* pair = old_pairs; pair < &old_pairs[old_alloc]; pair++)
    {
        if(!pair->key)
            continue;
        
        HashPair_T* new_pair = hashmap_find_pair(map, pair->key, true);
        assert(new_pair != 0);
        *new_pair = *pair;
    }

    free(old_pairs);
}

/*
 * This is an implementation of the well-documented Jenkins one-at-a-time
 * hash function. See https://en.wikipedia.org/wiki/Jenkins_hash_function
 */
size_t hashmap_default_hash(const char* data)
{
    size_t len = strlen(data);
    const u8* byte = (const u8*) data;

    size_t hash = 0;

    for(size_t i = 0; i < len; i++)
    {
        hash += *byte++;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

static inline size_t hashmap_calc_index(const HashMap_T* map, const char* key)
{
    size_t index = HASHMAP_HASH_FUNCTION(key);
    return HASHMAP_SIZE_MOD(map, index);
}

static inline HashPair_T* hashmap_find_pair(const HashMap_T* map, const char* key, bool find_empty)
{
    size_t index = hashmap_calc_index(map, key);

    // linear probing
    for(size_t i = 0; i < map->alloc; i++)
    {
        HashPair_T* pair = &map->pairs[index];
        if(!pair->key)
            return find_empty ? pair : NULL;
        
        if(strcmp(key, pair->key) == 0)
            return pair;
        
        index = HASHMAP_PROBE_NEXT(map, index);
    }

    return NULL;
}
