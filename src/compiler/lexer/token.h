#ifndef CSPYDR_TOKEN_H
#define CSPYDR_TOKEN_H

#include "../io/file.h"
#include "../globals.h"

typedef enum TOKEN_TYPE {
    TOKEN_ID,           // names, types, etc.
    TOKEN_MACRO_CALL,    // foo!, bar!()

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
    TOKEN_RANGE,        // ..
    TOKEN_ARROW,        // =>
    TOKEN_AND,          // &&
    TOKEN_OR,           // ||
    TOKEN_BIT_OR,       // |
    TOKEN_REF,          // &
    TOKEN_TILDE,        // ~

    TOKEN_MACRO_BEGIN,  // |:
    TOKEN_MACRO_END,    // :|

    TOKEN_INC,          // ++
    TOKEN_DEC,          // --

    TOKEN_ASSIGN,       // =
    TOKEN_ADD,          // +=
    TOKEN_SUB,          // -=
    TOKEN_MULT,         // *=
    TOKEN_DIV,          // /=

    TOKEN_COLON,        // :
    TOKEN_COMMA,        // ,
    TOKEN_DOT,          // .
    TOKEN_SEMICOLON,    // ;
    TOKEN_UNDERSCORE,   // _

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
    TOKEN_ENUM,         // enum
    TOKEN_IMPORT,       // import
    TOKEN_EXTERN,       // extern
    TOKEN_MACRO,        // macro
    TOKEN_CONST,        // const
    TOKEN_SIZEOF,       // sizeof
    TOKEN_TYPEOF,       // typeof
    TOKEN_WHILE,        // while
    TOKEN_FOR,          // for
    TOKEN_CONTINUE,     // continue
    TOKEN_BREAK,        // break
    TOKEN_NOOP,         // noop

    TOKEN_ERROR,        // error handling token
    TOKEN_EOF,          // end of file
} TokenType_T;

typedef struct TOKEN_STRUCT {
    char value[__CSP_MAX_TOKEN_SIZE];
    unsigned int line;
    unsigned int pos;
    TokenType_T type;

    SrcFile_T* source;
} __attribute__((packed)) Token_T;

Token_T* init_token(char* value, unsigned int line, unsigned int position, TokenType_T type, SrcFile_T* source);
void free_token(Token_T* token);

Token_T* dupl_token(Token_T* tok);  // duplicates a token

char* token_to_str(Token_T* token);

#endif