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

    TOKEN_EQUAL,            // =
    TOKEN_EQUAL_EQUAL,      // ==
    TOKEN_COLON,            // :
    TOKEN_COMMA,            // ,
    TOKEN_SEMICOLON,        // ;
    TOKEN_DOT,              // .
    TOKEN_GREATER,          // >
    TOKEN_GREATER_EQUALS,   // >=
    TOKEN_GREATER_GREATER,  // >>
    TOKEN_LESS,             // <
    TOKEN_LESS_EQUALS,      // <=
    TOKEN_LESS_LESS,        // <<
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
    TOKEN_AND,              // &
    TOKEN_AND_AND,          // &&
    TOKEN_AND_EQUALS,       // &=
    TOKEN_OR,               // |
    TOKEN_OR_OR,            // ||
    TOKEN_OR_EQUALS,        // |=

    TOKEN_STRING,           // "..."
    TOKEN_NUMBER,           // 1, 2, 3, ...
    TOKEN_TRUE,             // true
    TOKEN_FALSE,            // false

    TOKEN_LET,              // let
    TOKEN_INT,              // int
    TOKEN_FN,               // fn
    TOKEN_VEC,              // vec
    TOKEN_STR,              // str
    TOKEN_FLOAT,            // float
    TOKEN_BOOL,             // bool

    TOKEN_STMT,             //statement

    TOKEN_EOF       //end of file
} tokenType_T;

typedef struct TOKEN_STRUCT {
    char* value;
    tokenType_T type;
} token_T;

token_T* initToken(char* value, tokenType_T type);
char* tokenToString(token_T* token);
const char* tokenTypeToString(tokenType_T type);

#endif