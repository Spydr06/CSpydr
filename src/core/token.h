#ifndef CSPYDR_TOKEN_H
#define CSPYDR_TOKEN_H

typedef enum TOKENTYPE
{
    TOKEN_ID,               //names

    TOKEN_LEFT_PAREN,       // (
    TOKEN_RIGHT_PAREN,      // )
    TOKEN_LEFT_BRACE,       // {
    TOKEN_RIGHT_BRACE,      // }
    TOKEN_LEFT_BRACKET,     // [
    TOKEN_RIGHT_BRACKET,    // ]

    TOKEN_EQUALS,           // =
    TOKEN_EQUALS_EQUALS,    // ==
    TOKEN_EQUALS_GREATER,   // =>
    TOKEN_COLON,            // :
    TOKEN_COMMA,            // ,
    TOKEN_SEMICOLON,        // ;
    TOKEN_DOT,              // .
    TOKEN_GREATER,          // >
    TOKEN_GREATER_EQUALS,   // >=
    TOKEN_LESS,             // <
    TOKEN_LESS_EQUALS,      // <=
    TOKEN_PLUS,             // +
    TOKEN_PLUS_EQUALS,      // +=
    TOKEN_MINUS,            // -
    TOKEN_MINUS_EQUALS,     // -=
    TOKEN_STAR,             // *
    TOKEN_STAR_EQUALS,      // *=
    TOKEN_PERCENT,          // %
    TOKEN_PERCENT_EQUALS,   // %=
    TOKEN_SLASH,            // /
    TOKEN_SLASH_EQUALS,     // /=
    TOKEN_BANG,             // !
    TOKEN_BANG_EQUALS,      // !=
    TOKEN_AND_AND,          // &&
    TOKEN_OR_OR,            // ||
    TOKEN_UNDERSCORE,       // _

    TOKEN_NUMBER,           // 3, 3.14, 7.8, 0, ...
    TOKEN_STR,              // "abc..."
    TOKEN_CHAR,             // 'f'
    TOKEN_BOOL,             // true, false
    TOKEN_NIL,              // nil

    TOKEN_LET,              // let
    TOKEN_FN,               // fn

    TOKEN_IDENTIFIER,       // i32, i64, u32, u64, bool, str, char, vec
    TOKEN_STMT,             // statement

    TOKEN_EOF               //end of file
} tokenType_T;

typedef struct TOKEN_STRUCT {
    char* value;
    unsigned int line;
    tokenType_T type;
} token_T;

token_T* initToken(char* value, unsigned int line, tokenType_T type);
char* tokenToString(token_T* token);
const char* tokenTypeToString(tokenType_T type);

#endif

/*TOKEN_ID,           //names
    TOKEN_NUMBER,       // 0, 3, 5, 5.6, etc.
    TOKEN_CHAR,         // 'f'
    TOKEN_STRING,       // "foo"

    TOKEN_TRUE,         // true
    TOKEN_FALSE,        // false

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

    TOKEN_INC,          // ++
    TOKEN_DEC,          // --

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
    TOKEN_LOOP,         // loop
    TOKEN_FN,           // fn
    TOKEN_LET,          // let
    TOKEN_TYPE,         // type
    TOKEN_STRUCT,       // struct
    TOKEN_ENUM,         // enum
    TOKEN_INCLUDE,      // include*/