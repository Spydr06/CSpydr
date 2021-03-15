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

    TOKEN_VALUE,            // "...", ' ', true, false, nil, 3, 3.14, 7.8, 0, ...

    TOKEN_LET,              // let
    TOKEN_FN,               // fn

    TOKEN_IDENTIFIER,       // i8, i16, i32, i64, u8, u16, u32, u64, bool, str, char, vec
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