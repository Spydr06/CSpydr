#include "context.h"
#define PREPROCESSOR_TESTS  {"preprocessing simple file", test_preprocessing_simple_file}, \
                            {"preprocessing simple macro", test_processing_simple_macro},  \
                            {"preprocessing two macros", test_preprocessing_two_macros}

#include "lexer/token.h"
#include "parser/parser.h"
#include "preprocessor/preprocessor.h"

#define PREPROCESSOR_TEST_FUNC(name, src, code)  \
    void name(void) {                            \
        Context_T context;                       \
        init_context(&context);                  \
        context.flags.silent = true;             \
        ASTProg_T prog = {0};                    \
        initialization_pass(&context, &prog);    \
        list_push(prog.files, get_file(1, src)); \
        lexer_pass(&context, &prog);             \
        preprocessor_pass(&context, &prog);      \
        List_T* tokens = prog.tokens;            \
        { code }                                 \
    }

PREPROCESSOR_TEST_FUNC(test_preprocessing_simple_file, "fn main(): i32 {}",
    TEST_ASSERT(tokens->size == 9);

    TEST_ASSERT(((Token_T*) tokens->items[0])->type == TOKEN_FN);
    TEST_ASSERT(((Token_T*) tokens->items[1])->type == TOKEN_ID);
    TEST_ASSERT(((Token_T*) tokens->items[2])->type == TOKEN_LPAREN);
    TEST_ASSERT(((Token_T*) tokens->items[3])->type == TOKEN_RPAREN);
    TEST_ASSERT(((Token_T*) tokens->items[4])->type == TOKEN_OPERATOR);
    TEST_ASSERT(((Token_T*) tokens->items[5])->type == TOKEN_ID);
    TEST_ASSERT(((Token_T*) tokens->items[6])->type == TOKEN_LBRACE);
    TEST_ASSERT(((Token_T*) tokens->items[7])->type == TOKEN_RBRACE);
    TEST_ASSERT(((Token_T*) tokens->items[8])->type == TOKEN_EOF);
)

PREPROCESSOR_TEST_FUNC(test_processing_simple_macro, "macro foo { bar } foo!",
    TEST_ASSERT(tokens->size == 2);

    TEST_ASSERT(((Token_T*) tokens->items[0])->type == TOKEN_ID);
    TEST_ASSERT(((Token_T*) tokens->items[1])->type == TOKEN_EOF);
)

PREPROCESSOR_TEST_FUNC(test_preprocessing_two_macros, "macro foo { bar } \nmacro bar { 4 } bar! foo!",
    TEST_ASSERT(tokens->size == 3);

    TEST_ASSERT(((Token_T*) tokens->items[0])->type == TOKEN_INT);
    TEST_ASSERT(((Token_T*) tokens->items[1])->type == TOKEN_ID);
    TEST_ASSERT(((Token_T*) tokens->items[2])->type == TOKEN_EOF);
)
