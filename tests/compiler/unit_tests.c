#include <acutest/include/acutest.h>

#include "lexer/lexer.h"
#include "preprocessor/preprocessor.h"
#include "parser/parser.h"
#include "globals.h"
#include <stdarg.h>
#include <string.h>

static File_T* get_file(int num_lines, ...) 
{
    List_T* lines = init_list();
    va_list va;

    va_start(va, num_lines);
    for(int i = 0; i < num_lines; i++)
        list_push(lines, va_arg(va, char*));
    va_end(va);

    return init_file(lines, "generated");
}

void test_file_generation(void)
{
    File_T* file = get_file(2, "hello", "world");
    TEST_ASSERT(file != NULL);

    TEST_ASSERT(file->path != NULL);
    TEST_CHECK(strcmp(file->path, "generated") == 0);

    TEST_ASSERT(file->num_lines = 2);
    TEST_ASSERT(file->lines != NULL);
    
    TEST_CHECK(strcmp(file->lines->items[0], "hello") == 0);
    TEST_CHECK(strcmp(file->lines->items[1], "world") == 0);
}

#include "test_lexer.h"
#include "test_preprocessor.h"
#include "test_parser.h"

TEST_LIST = {
   {"file generation", test_file_generation},
   LEXER_TESTS,         // all lexer tests included from "test_lexer.tt"
   PREPROCESSOR_TESTS,   // all preprocessor tests included from "test_preprocessor.tt"
   PARSER_TESTS,        // all parser tests included from "test_parser.tt"
   {NULL, NULL}         // end of the tests
};