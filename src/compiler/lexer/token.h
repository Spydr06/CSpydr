#ifndef CSPYDR_TOKEN_H
#define CSPYDR_TOKEN_H

#include "../io/file.h"
#include "../globals.h"

typedef enum TOKEN_TYPE {
    TOKEN_ID,           // names, types, etc.
    TOKEN_MACRO_CALL,   // foo!, bar!()
    TOKEN_INFIX_CALL,   // `

    TOKEN_INT,          // 0, 3, 5, etc.
    TOKEN_FLOAT,        // 4.2, 3.14, etc.
    TOKEN_CHAR,         // 'f'
    TOKEN_STRING,       // "foo"

    TOKEN_TRUE,         // true
    TOKEN_FALSE,        // false
    TOKEN_NIL,          // nil (null)

    TOKEN_LPAREN,       // (
    TOKEN_RPAREN,       // )
    TOKEN_LBRACE,       // {
    TOKEN_RBRACE,       // }
    TOKEN_LBRACKET,     // [
    TOKEN_RBRACKET,     // ]

    TOKEN_GT,           // >
    TOKEN_LT,           // <
    TOKEN_EQ,           // ==
    TOKEN_NOT_EQ,       // !=
    TOKEN_GT_EQ,        // >=
    TOKEN_LT_EQ,        // <=
    TOKEN_BANG,         // !

    TOKEN_PLUS,         // +
    TOKEN_MINUS,        // -
    TOKEN_STAR,         // *
    TOKEN_SLASH,        // /
    TOKEN_PERCENT,      // %
    TOKEN_RANGE,        // ..
    TOKEN_ARROW,        // =>
    TOKEN_AND,          // &&
    TOKEN_OR,           // ||
    TOKEN_BIT_OR,       // |
    TOKEN_REF,          // &
    TOKEN_TILDE,        // ~
    TOKEN_VA_LIST,      // ...

    TOKEN_INC,          // ++
    TOKEN_DEC,          // --

    TOKEN_ASSIGN,       // =
    TOKEN_ADD,          // +=
    TOKEN_SUB,          // -=
    TOKEN_MULT,         // *=
    TOKEN_DIV,          // /=
    TOKEN_MOD,          // %=

    TOKEN_XOR,          // ^
    TOKEN_XOR_ASSIGN,   // ^=
    TOKEN_LSHIFT,       // <<
    TOKEN_RSHIFT,       // >>
    
    TOKEN_LSHIFT_ASSIGN,// <<=
    TOKEN_RSHIFT_ASSIGN,// >>=

    TOKEN_BIT_AND_ASSIGN,// &=
    TOKEN_BIT_OR_ASSIGN, // |=

    TOKEN_COLON,        // :
    TOKEN_COMMA,        // ,
    TOKEN_DOT,          // .
    TOKEN_SEMICOLON,    // ;
    TOKEN_UNDERSCORE,   // _

    TOKEN_STATIC_MEMBER,// ::

    TOKEN_POW_2,        // ²
    TOKEN_POW_3,        // ³

    TOKEN_AT,           // @
    TOKEN_DOLLAR,       // $

    TOKEN_IF,           // if
    TOKEN_ELSE,         // else
    TOKEN_LOOP,         // loop
    TOKEN_RETURN,       // ret
    TOKEN_MATCH,        // match
    TOKEN_FN,           // fn
    TOKEN_LET,          // let
    TOKEN_TYPE,         // type
    TOKEN_STRUCT,       // struct
    TOKEN_UNION,        // union
    TOKEN_ENUM,         // enum
    TOKEN_IMPORT,       // import
    TOKEN_EXTERN,       // extern
    TOKEN_MACRO,        // macro
    TOKEN_CONST,        // const
    TOKEN_NAMESPACE,    // namespace
    TOKEN_COMPLEX,      // complex
    TOKEN_ATOMIC,       // atomic
    TOKEN_VOLATILE,     // volatile
    TOKEN_SIZEOF,       // sizeof
    TOKEN_TYPEOF,       // typeof
    TOKEN_ALIGNOF,      // alignof
    TOKEN_WHILE,        // while
    TOKEN_FOR,          // for
    TOKEN_CONTINUE,     // continue
    TOKEN_BREAK,        // break
    TOKEN_NOOP,         // noop
    TOKEN_LEN,          // len
    TOKEN_VA_ARG,       // va_arg
    TOKEN_ASM,          // asm

    TOKEN_CURRENT_FN,   // special token for the __func__! macro

    TOKEN_ERROR,        // error handling token
    TOKEN_EOF,          // end of file
} TokenType_T;

typedef struct TOKEN_STRUCT {
    char value[__CSP_MAX_TOKEN_SIZE];
    u32 line;
    u32 pos;
    TokenType_T type;

    SrcFile_T* source;
} __attribute__((packed)) Token_T;

Token_T* init_token(char* value, u32 line, u32 position, TokenType_T type, SrcFile_T* source);
char* token_to_str(Token_T* token);

#endif