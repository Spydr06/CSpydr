#ifndef __CSPYDR_TOKENS_H
#define __CSPYDR_TOKENS_H

enum CSPYDR_TOKEN_TYPE {
    CSPYDR_TOKEN_ID,           // names, types, etc.
    CSPYDR_TOKEN_MACRO_CALL,   // foo!
    CSPYDR_TOKEN_INFIX_CALL,   // `

    CSPYDR_TOKEN_INT,          // 0, 3, 5, etc.
    CSPYDR_TOKEN_FLOAT,        // 4.2, 3.14, etc.
    CSPYDR_TOKEN_CHAR,         // 'f'
    CSPYDR_TOKEN_STRING,       // "foo"

    CSPYDR_TOKEN_TRUE,         // true
    CSPYDR_TOKEN_FALSE,        // false
    CSPYDR_TOKEN_NIL,          // nil

    CSPYDR_TOKEN_LPAREN,       // (
    CSPYDR_TOKEN_RPAREN,       // )
    CSPYDR_TOKEN_LBRACE,       // {
    CSPYDR_TOKEN_RBRACE,       // }
    CSPYDR_TOKEN_LBRACKET,     // [
    CSPYDR_TOKEN_RBRACKET,     // ]

    CSPYDR_TOKEN_GT,           // >
    CSPYDR_TOKEN_LT,           // <
    CSPYDR_TOKEN_EQ,           // ==
    CSPYDR_TOKEN_NOT_EQ,       // !=
    CSPYDR_TOKEN_GT_EQ,        // >=
    CSPYDR_TOKEN_LT_EQ,        // <=
    CSPYDR_TOKEN_BANG,         // !

    CSPYDR_TOKEN_PLUS,         // +
    CSPYDR_TOKEN_MINUS,        // -
    CSPYDR_TOKEN_STAR,         // *
    CSPYDR_TOKEN_SLASH,        // /
    CSPYDR_TOKEN_PERCENT,      // %
    CSPYDR_TOKEN_ARROW,        // =>
    CSPYDR_TOKEN_AND,          // &&
    CSPYDR_TOKEN_OR,           // ||
    CSPYDR_TOKEN_BIT_OR,       // |
    CSPYDR_TOKEN_PIPE,         // |>
    CSPYDR_TOKEN_REF,          // &
    CSPYDR_TOKEN_TILDE,        // ~
    CSPYDR_TOKEN_VA_LIST,      // ...

    CSPYDR_TOKEN_INC,          // ++
    CSPYDR_TOKEN_DEC,          // --

    CSPYDR_TOKEN_ASSIGN,       // =
    CSPYDR_TOKEN_ADD,          // +=
    CSPYDR_TOKEN_SUB,          // -=
    CSPYDR_TOKEN_MULT,         // *=
    CSPYDR_TOKEN_DIV,          // /=
    CSPYDR_TOKEN_MOD,          // %=

    CSPYDR_TOKEN_XOR,          // ^
    CSPYDR_TOKEN_XOR_ASSIGN,   // ^=
    CSPYDR_TOKEN_LSHIFT,       // <<
    CSPYDR_TOKEN_RSHIFT,       // >>
    
    CSPYDR_TOKEN_LSHIFT_ASSIGN,// <<=
    CSPYDR_TOKEN_RSHIFT_ASSIGN,// >>=

    CSPYDR_TOKEN_BIT_AND_ASSIGN,// &=
    CSPYDR_TOKEN_BIT_OR_ASSIGN, // |=

    CSPYDR_TOKEN_COLON,        // :
    CSPYDR_TOKEN_COMMA,        // ,
    CSPYDR_TOKEN_DOT,          // .
    CSPYDR_TOKEN_SEMICOLON,    // ;
    CSPYDR_TOKEN_UNDERSCORE,   // _
    CSPYDR_TOKEN_DOLLAR,       // $

    CSPYDR_TOKEN_STATIC_MEMBER,// ::

    CSPYDR_TOKEN_POW_2,        // ²
    CSPYDR_TOKEN_POW_3,        // ³

    CSPYDR_TOKEN_C_ARRAY,      // 'c or 'C

    CSPYDR_TOKEN_IF,           // if
    CSPYDR_TOKEN_ELSE,         // else
    CSPYDR_TOKEN_LOOP,         // loop
    CSPYDR_TOKEN_RETURN,       // ret
    CSPYDR_TOKEN_MATCH,        // match
    CSPYDR_TOKEN_FN,           // fn
    CSPYDR_TOKEN_LET,          // let
    CSPYDR_TOKEN_TYPE,         // type
    CSPYDR_TOKEN_STRUCT,       // struct
    CSPYDR_TOKEN_UNION,        // union
    CSPYDR_TOKEN_ENUM,         // enum
    CSPYDR_TOKEN_IMPORT,       // import
    CSPYDR_TOKEN_EXTERN,       // extern
    CSPYDR_TOKEN_MACRO,        // macro
    CSPYDR_TOKEN_CONST,        // const
    CSPYDR_TOKEN_NAMESPACE,    // namespace
    CSPYDR_TOKEN_SIZEOF,       // sizeof
    CSPYDR_TOKEN_TYPEOF,       // typeof
    CSPYDR_TOKEN_ALIGNOF,      // alignof
    CSPYDR_TOKEN_WHILE,        // while
    CSPYDR_TOKEN_FOR,          // for
    CSPYDR_TOKEN_CONTINUE,     // continue
    CSPYDR_TOKEN_BREAK,        // break
    CSPYDR_TOKEN_NOOP,         // noop
    CSPYDR_TOKEN_LEN,          // len
    CSPYDR_TOKEN_ASM,          // asm
    CSPYDR_TOKEN_USING,        // using
    CSPYDR_TOKEN_WITH,         // with
    CSPYDR_TOKEN_DO,           // do
    CSPYDR_TOKEN_UNLESS,       // unless

    CSPYDR_TOKEN_CURRENT_FN,   // special token for the __func__! macro

    // builtin functions used exclusively in type expressions
    CSPYDR_TOKEN_BUILTIN_REG_CLASS,
    CSPYDR_TOKEN_BUILTIN_IS_INT,
    CSPYDR_TOKEN_BUILTIN_IS_UINT,
    CSPYDR_TOKEN_BUILTIN_IS_FLOAT,
    CSPYDR_TOKEN_BUILTIN_IS_POINTER,
    CSPYDR_TOKEN_BUILTIN_IS_ARRAY,
    CSPYDR_TOKEN_BUILTIN_IS_STRUCT,
    CSPYDR_TOKEN_BUILTIN_IS_UNION,
    CSPYDR_TOKEN_BUILTIN_TO_STR,

    CSPYDR_TOKEN_ERROR, // error handling token
    CSPYDR_TOKEN_EOF,   // end of file
};

#endif // __CSPYDR_TOKENS_H