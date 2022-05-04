#define HASHMAP_TESTS                      \
    {"hashmap_init()", test_hashmap_init}, \
    {"hashmap_free()", test_hashmap_free}, \
    {"hashmap_put()", test_hashmap_put},   \
    {"hashmap_get()", test_hashmap_get},   \
    {"hashmap_grow()", test_hashmap_grow}

#include <hashmap.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void test_hashmap_init(void) 
{
    HashMap_T* map = hashmap_init();
    TEST_ASSERT(map != NULL);
}

void test_hashmap_free(void) 
{
    HashMap_T* map = hashmap_init();
    TEST_ASSERT(map != NULL);
    hashmap_free(map);
}

void test_hashmap_put(void)
{
    HashMap_T* map = hashmap_init();
    TEST_ASSERT(map != NULL);

    int err = hashmap_put(map, "1", "1");
    TEST_ASSERT(err == 0);

    hashmap_free(map);
}

void test_hashmap_get(void)
{
    HashMap_T* map = hashmap_init();
    TEST_ASSERT(map != NULL);

    int err = hashmap_put(map, "1", "one");
    TEST_ASSERT(err == 0);
    err = hashmap_put(map, "2", "two");
    TEST_ASSERT(err == 0);
    err = hashmap_put(map, "3", "three");
    TEST_ASSERT(err == 0);

    char* val = hashmap_get(map, "1");
    TEST_ASSERT(val != NULL);
    TEST_ASSERT(strcmp(val, "one") == 0);

    val = hashmap_get(map, "2");
    TEST_ASSERT(val != NULL);
    TEST_ASSERT(strcmp(val, "two") == 0);

    val = hashmap_get(map, "3");
    TEST_ASSERT(val != NULL);
    TEST_ASSERT(strcmp(val, "three") == 0);

    hashmap_free(map); 
}

void test_hashmap_grow(void)
{
    HashMap_T* map = hashmap_init();
    TEST_ASSERT(map != NULL);

    for(int i = 0; i < 514; i++)
    {
        char* str = calloc(10, sizeof(char));
        sprintf(str, "%d", i);
        int err = hashmap_put(map, str, "hi");
        TEST_ASSERT(err == 0);
    }

    char* val = hashmap_get(map, "51");
    TEST_ASSERT(val != NULL);
    TEST_ASSERT(strcmp(val, "hi") == 0);

    hashmap_free(map);
}