#include "lexer.h"
#include "token.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#define MAX(a, b) a > b ? a : b
#define MIN(a, b) a < b ? a : b

lexer_T* initLexer(char* src) 
{
    lexer_T* lexer = calloc(1, sizeof(struct LEXER_STRUCT));
    lexer->src = src;
    lexer->srcSize = strlen(src);
    lexer->i = 0;
    lexer->c = src[lexer->i];

    return lexer;
}

void lexerAdvance(lexer_T* lexer)
{
    if(lexer->i < lexer->srcSize && lexer->c != '\0')
    {
        lexer->i++;
        lexer->c = lexer->src[lexer->i];
    }
}

token_T* lexerAdvanceWith(lexer_T* lexer, token_T* token)
{
    lexerAdvance(lexer);
    return token;
}

token_T* lexerAdvanceCurrent(lexer_T* lexer, tokenType_T type)
{
    char* value = calloc(2, sizeof(char)); 
    value[0] = lexer->c;
    value[1] = '\0';

    token_T* token = initToken(value, type);
    lexerAdvance(lexer);

    return token;
}

void lexerSkipWhitespace(lexer_T* lexer)
{
    while(lexer->c == 13 || lexer->c == 10 || lexer->c == ' ' || lexer->c == '\t') {
        lexerAdvance(lexer);
    }
}

char lexerPeek(lexer_T* lexer, int offset)
{
    return lexer->src[MIN(lexer->i + offset, lexer->srcSize)];
}

token_T* lexerParseId(lexer_T* lexer)
{
    char* value = calloc(1, sizeof(char));
    while(isalnum(lexer->c)) 
    {
        value = realloc(value, (strlen(value) + 2) * sizeof(char));
        strcat(value, (char[]){lexer->c, 0});
        lexerAdvance(lexer);
    }
    tokenType_T type = TOKEN_ID;

    if(strcmp(value, "true") == 0) {
        type = TOKEN_TRUE;
    }
    else if(strcmp(value, "false") == 0) {
        type = TOKEN_FALSE;
    }
    else if(strcmp(value, "let") == 0) {
        type = TOKEN_LET;
    }
    else if(strcmp(value, "int") == 0) {
        type = TOKEN_INT;
    }
    else if(strcmp(value, "fn") == 0) {
        type = TOKEN_FN;
    }
    else if(strcmp(value, "vec") == 0) {
        type = TOKEN_VEC;
    }
    else if(strcmp(value, "str") == 0) {
        type = TOKEN_STR;
    }
    else if(strcmp(value, "float") == 0) {
        type = TOKEN_FLOAT;
    }
    else if(strcmp(value, "bool") == 0) {
        type = TOKEN_BOOL;
    }
    else if(strcmp(value, "if") == 0) {
        type = TOKEN_STMT;
    }
    else if(strcmp(value, "for") == 0) {
        type = TOKEN_STMT;
    }
    else if(strcmp(value, "while") == 0) {
        type = TOKEN_STMT;
    }
    else if(strcmp(value, "loop") == 0) {
        type = TOKEN_STMT;
    }
    else if(strcmp(value, "else") == 0) {
        type = TOKEN_STMT;
    }
    else if(strcmp(value, "elif") == 0) {
        type = TOKEN_STMT;
    }
    else if(strcmp(value, "exit") == 0) {
        type = TOKEN_STMT;
    }

    return initToken(value, type);
}

token_T* lexerParseNumber(lexer_T* lexer)
{
    char* value = calloc(1, sizeof(char));

    while(isdigit(lexer->c) || lexer->c == '.') 
    {
        value = realloc(value, (strlen(value) + 2) * sizeof(char));
        strcat(value, (char[]){lexer->c, 0});
        lexerAdvance(lexer);
    }

    return initToken(value, TOKEN_NUMBER);
}

token_T* lexerParseString(lexer_T* lexer)
{
    lexerAdvance(lexer);
    char* value = calloc(1, sizeof(char));
    
    while(lexer->c != '"')
    {
        value = realloc(value, (strlen(value) + 2) * sizeof(char));
        strcat(value, (char[]){lexer->c, 0});
        lexerAdvance(lexer);

        if(lexer->c == '\0') {
            fprintf(stderr, "[SYNTAX ERROR] LEXER: Unterminated string!");
            exit(1);
        }
    }
    lexerAdvance(lexer);

    return initToken(value, TOKEN_STRING);
}

token_T* lexerNextToken(lexer_T* lexer)
{
    while(lexer->c != '\0') {
        lexerSkipWhitespace(lexer);

        if(isalpha(lexer->c)) {
            return lexerParseId(lexer);
        }

        if(isdigit(lexer->c)) {
            return lexerParseNumber(lexer);
        }

        switch (lexer->c) {
            case '(': return lexerAdvanceCurrent(lexer, TOKEN_LEFT_PAREN);
            case ')': return lexerAdvanceCurrent(lexer, TOKEN_RIGHT_PAREN);
            case '{': return lexerAdvanceCurrent(lexer, TOKEN_LEFT_BRACE);
            case '}': return lexerAdvanceCurrent(lexer, TOKEN_RIGHT_BRACE);
            case '[': return lexerAdvanceCurrent(lexer, TOKEN_LEFT_BRACKET);
            case ']': return lexerAdvanceCurrent(lexer, TOKEN_RIGHT_BRACKET);

            case '=': {
                if(lexerPeek(lexer, 1) == '=') 
                    return lexerAdvanceWith(lexer, lexerAdvanceWith(lexer, initToken("==", TOKEN_EQUAL_EQUAL)));
                else 
                    return lexerAdvanceWith(lexer, initToken("=", TOKEN_EQUAL));
            } break;
            case ':': return lexerAdvanceCurrent(lexer, TOKEN_COLON);
            case ';': return lexerAdvanceCurrent(lexer, TOKEN_SEMICOLON);
            case '.': return lexerAdvanceCurrent(lexer, TOKEN_DOT);
            case ',': return lexerAdvanceCurrent(lexer, TOKEN_COMMA);
            case '>': {
                if(lexerPeek(lexer, 1) == '=') 
                    return lexerAdvanceWith(lexer, lexerAdvanceWith(lexer, initToken(">=", TOKEN_GREATER_EQUALS)));
                else if(lexerPeek(lexer, 1) == '>')
                    return lexerAdvanceWith(lexer, lexerAdvanceWith(lexer, initToken(">>", TOKEN_GREATER_GREATER)));
                else 
                    return lexerAdvanceWith(lexer, initToken(">", TOKEN_GREATER));
            } break;
            case '<': {
                if(lexerPeek(lexer, 1) == '=') 
                    return lexerAdvanceWith(lexer, lexerAdvanceWith(lexer, initToken("<=", TOKEN_LESS_EQUALS)));
                else if(lexerPeek(lexer, 1) == '<')
                    return lexerAdvanceWith(lexer, lexerAdvanceWith(lexer, initToken("<<", TOKEN_LESS_LESS)));
                else if(lexerPeek(lexer, 1) == '-')
                    return lexerAdvanceWith(lexer, lexerAdvanceWith(lexer, initToken("return", TOKEN_STMT)));
                else 
                    return lexerAdvanceWith(lexer, initToken("<", TOKEN_LESS));
            } break;
            case '+': {
                if(lexerPeek(lexer, 1) == '=') 
                    return lexerAdvanceWith(lexer, lexerAdvanceWith(lexer, initToken("+=", TOKEN_PLUS_EQUALS)));
                else 
                    return lexerAdvanceWith(lexer, initToken("+", TOKEN_PLUS));
            } break;
            case '-': {
                if(lexerPeek(lexer, 1) == '=') 
                    return lexerAdvanceWith(lexer, lexerAdvanceWith(lexer, initToken("-=", TOKEN_MINUS_EQUALS)));
                else 
                    return lexerAdvanceWith(lexer, initToken("-", TOKEN_MINUS));
            } break;
            case '*': {
                if(lexerPeek(lexer, 1) == '=') 
                    return lexerAdvanceWith(lexer, lexerAdvanceWith(lexer, initToken("*=", TOKEN_STAR_EQUALS)));
                else 
                    return lexerAdvanceWith(lexer, initToken("*", TOKEN_STAR));
            } break;
            case '%': {
                if(lexerPeek(lexer, 1) == '=') 
                    return lexerAdvanceWith(lexer, lexerAdvanceWith(lexer, initToken("%%=", TOKEN_PERCENT_EQUALS)));
                else 
                    return lexerAdvanceWith(lexer, initToken("%%", TOKEN_PERCENT));
            } break;
            case '/': {
                if(lexerPeek(lexer, 1) == '=') 
                    return lexerAdvanceWith(lexer, lexerAdvanceWith(lexer, initToken("/=", TOKEN_SLASH_EQUALS)));
                else 
                    return lexerAdvanceWith(lexer, initToken("/", TOKEN_SLASH));
            } break;
            case '!': {
                if(lexerPeek(lexer, 1) == '=') 
                    return lexerAdvanceWith(lexer, lexerAdvanceWith(lexer, initToken("!=", TOKEN_BANG_EQUALS)));
                else 
                    return lexerAdvanceWith(lexer, initToken("!", TOKEN_BANG));
            } break;
            case '&': {
                if(lexerPeek(lexer, 1) == '=') 
                    return lexerAdvanceWith(lexer, lexerAdvanceWith(lexer, initToken("&=", TOKEN_AND_EQUALS)));
                else if(lexerPeek(lexer, 1) == '&')
                    return lexerAdvanceWith(lexer, initToken("&&", TOKEN_AND_AND));
                else 
                    return lexerAdvanceWith(lexer, initToken("&", TOKEN_AND));
            } break;
            case '|': {
                if(lexerPeek(lexer, 1) == '=') 
                    return lexerAdvanceWith(lexer, lexerAdvanceWith(lexer, initToken("|=", TOKEN_OR_EQUALS)));
                else if(lexerPeek(lexer, 1) == '&')
                    return lexerAdvanceWith(lexer, initToken("||", TOKEN_OR_OR));
                else 
                    return lexerAdvanceWith(lexer, initToken("|", TOKEN_OR));
            } break;

            case '\"':
                return lexerParseString(lexer);

            case '\0': break;
            default: 
                printf("[Lexer]: Unexpected character '%d'\n", lexer->c);
                exit(1);
                break;
        }
    }

    return initToken(0, TOKEN_EOF);
}