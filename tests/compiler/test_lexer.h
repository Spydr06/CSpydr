#include "config.h"
#include "context.h"
#define LEXER_TESTS                             \
   /* {"lexing symbols", test_lexer_symbols},  */    \
    {"lexing strings", test_lexer_strings},     \
    {"lexing numbers", test_lexer_numbers},     \
    {"lexing ids", test_lexer_ids},             \
    {"lexing keywords", test_lexer_keywords}

#include <string.h>

#include "lexer/token.h"
#include "lexer/lexer.h"

#ifndef LEN
#define LEN(arr) (sizeof(arr) / sizeof(*arr))
#endif

void check_tokens_str(char** expected_tokens, File_T* file, int num_tokens, bool heap_value)
{
    Context_T context;
    init_context(&context);
    Lexer_T lexer;
    init_lexer(&lexer, &context, file);

    for(int i = 0; i < num_tokens; i++)
    {
        Token_T* token =  lexer_next_token(&lexer);

        if(heap_value)
        {
            TEST_CHECK(token->value != NULL);
            TEST_CHECK(strcmp(token->value, expected_tokens[i]) == 0);
        }
        else
        {
            TEST_CHECK(strcmp(token->value, expected_tokens[i]) == 0);
        }
    }
}

void check_tokens(TokenType_T* expected_tokens, File_T* file, int num_tokens)
{
    Context_T context;
    init_context(&context);
    Lexer_T lexer;
    init_lexer(&lexer, &context, file);
    for(int i = 0; i < num_tokens; i++)
    {
        Token_T* token =  lexer_next_token(&lexer);
        TEST_ASSERT(token != NULL);
        TEST_CHECK(token->type == expected_tokens[i]);

        TEST_ASSERT(token->value != NULL);
    }
}

/*void test_lexer_symbols(void)
{
    File_T* file = get_file(1, "++ += + -- -= - *= * %= % /= / &= && & ^= ^ <<= << >>= >> || |= |> | == => = != ! >= > <= <- < ( ) { } [ ] ~ , ; ; _ :: : ... . ² ³ $ `");
    TEST_ASSERT(file != NULL);
    
    TokenType_T expected_tokens[] = {
        TOKEN_INC,
        TOKEN_ADD,
        TOKEN_PLUS,
        TOKEN_DEC,
        TOKEN_SUB,
        TOKEN_MINUS,
        TOKEN_MULT,
        TOKEN_STAR,
        TOKEN_MOD,
        TOKEN_PERCENT,
        TOKEN_DIV,
        TOKEN_SLASH,
        TOKEN_BIT_AND_ASSIGN,
        TOKEN_AND,
        TOKEN_REF,
        TOKEN_XOR_ASSIGN,
        TOKEN_XOR,
        TOKEN_LSHIFT_ASSIGN,
        TOKEN_LSHIFT,
        TOKEN_RSHIFT_ASSIGN,
        TOKEN_RSHIFT,
        TOKEN_OR,
        TOKEN_BIT_OR_ASSIGN,
        TOKEN_PIPE,
        TOKEN_BIT_OR,
        TOKEN_EQ,
        TOKEN_ARROW,
        TOKEN_ASSIGN,
        TOKEN_NOT_EQ,
        TOKEN_BANG,
        TOKEN_GT_EQ,
        TOKEN_GT,
        TOKEN_LT_EQ,
        TOKEN_RETURN,
        TOKEN_LT,
        TOKEN_LPAREN,
        TOKEN_RPAREN,
        TOKEN_LBRACE,
        TOKEN_RBRACE,
        TOKEN_LBRACKET,
        TOKEN_RBRACKET,
        TOKEN_TILDE,
        TOKEN_COMMA,
        TOKEN_SEMICOLON,
        TOKEN_SEMICOLON,
        TOKEN_UNDERSCORE,
        TOKEN_STATIC_MEMBER,
        TOKEN_COLON,
        TOKEN_VA_LIST,
        TOKEN_DOT,
        TOKEN_POW_2,
        TOKEN_POW_3,
        TOKEN_DOLLAR,
        TOKEN_INFIX_CALL
    };
    
    check_tokens(expected_tokens, file, LEN(expected_tokens));
}*/

void test_lexer_strings(void)
{
    File_T* file = get_file(4, "\"hello\"\n", "\"\n", "world\n", "\"\n");
    TEST_ASSERT(file != NULL);

    char* expected_tokens[] = {
        "hello", "\nworld\n"
    };

    check_tokens_str(expected_tokens, file, LEN(expected_tokens), true);
}

void test_lexer_numbers(void)
{
    File_T* file = get_file(1, "23 2.5 0b0101 0xff 0o10 100_000_000");
    TEST_ASSERT(file != NULL);

    char* expected_tokens[] = {
        "23", "2.5", "5", "255", "8", "100000000"
    };

    check_tokens_str(expected_tokens, file, LEN(expected_tokens), false);
}

void test_lexer_ids(void)
{
    File_T* file = get_file(2, "hello wor\n", "ld");
    TEST_ASSERT(file != NULL);

    char* expected_tokens[] = {
        "hello", "wor", "ld"
    };

    check_tokens_str(expected_tokens, file, LEN(expected_tokens), false);
}

void test_lexer_keywords(void)
{
    File_T* file = get_file(1, "true false nil let fn loop while for if else ret match type struct union enum import const extern macro namespace sizeof typeof alignof break continue noop len asm using with");
    TEST_ASSERT(file != NULL);

    TokenType_T expected_tokens[] = {
        TOKEN_TRUE,
        TOKEN_FALSE,
        TOKEN_NIL,
        TOKEN_LET,
        TOKEN_FN,
        TOKEN_LOOP,
        TOKEN_WHILE,
        TOKEN_FOR,
        TOKEN_IF,
        TOKEN_ELSE,
        TOKEN_RETURN,
        TOKEN_MATCH,
        TOKEN_TYPE,
        TOKEN_STRUCT,
        TOKEN_UNION,
        TOKEN_ENUM,
        TOKEN_IMPORT,
        TOKEN_CONST,
        TOKEN_EXTERN,
        TOKEN_MACRO,
        TOKEN_NAMESPACE,
        TOKEN_SIZEOF,
        TOKEN_TYPEOF,
        TOKEN_ALIGNOF,
        TOKEN_BREAK,
        TOKEN_CONTINUE,
        TOKEN_NOOP,
        TOKEN_LEN,
        TOKEN_ASM,
        TOKEN_USING,
        TOKEN_WITH
    };

    check_tokens(expected_tokens, file, LEN(expected_tokens));
}
