#include <acutest/include/acutest.h>

#include "../../src/compiler/lexer/lexer.h"
#include "../../src/compiler/lexer/preprocessor.h"
#include "../../src/compiler/parser/parser.h"
#include <stdarg.h>
#include <string.h>

static SrcFile_T* get_file(int num_lines, ...) 
{
    List_T* lines = init_list(sizeof(char*));
    va_list va;

    va_start(va, num_lines);
    for(int i = 0; i < num_lines; i++)
        list_push(lines, va_arg(va, char*));
    va_end(va);

    return init_srcfile(lines, "generated");
}

void test_file_generation(void)
{
    SrcFile_T* file = get_file(2, "hello", "world");
    TEST_ASSERT(file != NULL);

    TEST_ASSERT(file->path != NULL);
    TEST_CHECK(strcmp(file->path, "generated") == 0);

    TEST_ASSERT(file->num_lines = 2);
    TEST_ASSERT(file->lines != NULL);
    
    TEST_CHECK(strcmp(file->lines->items[0], "hello") == 0);
    TEST_CHECK(strcmp(file->lines->items[1], "world") == 0);
}

#include "test_lexer.tt"
#include "test_preprocessor.tt"
#include "test_parser.tt"

TEST_LIST = {
   {"file generation", test_file_generation},
   LEXER_TESTS,         // all lexer tests included from "test_lexer.tt"
   PREPROCESSOR_TESTS,   // all preprocessor tests included from "test_preprocessor.tt"
   PARSER_TESTS,        // all parser tests included from "test_parser.tt"
   {NULL, NULL}         // end of the tests
};