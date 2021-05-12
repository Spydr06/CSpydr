#include "lib/acutest.h"

#include "../src/compiler/lexer/lexer.h"
#include <stdarg.h>
#include <string.h>

srcFile_T* getFile(int numLines, ...) 
{
    list_T* lines = initList(sizeof(char*));
    va_list va;

    va_start(va, numLines);
    for(int i = 0; i < numLines; i++)
        listPush(lines, va_arg(va, char*));
    va_end(va);

    return initSrcFile(lines, "generated");
}

void test_file_generation(void)
{
    srcFile_T* file = getFile(2, "hello", "world");
    TEST_ASSERT(file != NULL);

    TEST_ASSERT(file->path != NULL);
    TEST_CHECK(strcmp(file->path, "generated") == 0);

    TEST_ASSERT(file->numLines = 2);
    TEST_ASSERT(file->lines != NULL);
    
    TEST_CHECK(strcmp(file->lines->items[0], "hello") == 0);
    TEST_CHECK(strcmp(file->lines->items[1], "world") == 0);
}

#include "test_lexer.tt"

TEST_LIST = {
   {"file generation", test_file_generation},
   LEXER_TESTS,
   {NULL, NULL}     /* zeroed record marking the end of the list */
};