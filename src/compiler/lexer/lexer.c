#include "lexer.h"
#include "token.h"

#include "../io/log.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_KEYWORDS 15

const struct { const char* str; TokenType_T type; } keywords[NUM_KEYWORDS] = {
    {"true", TOKEN_TRUE},
    {"false", TOKEN_FALSE},
    {"nil", TOKEN_NIL},
    {"let", TOKEN_LET},
    {"fn", TOKEN_FN},
    {"loop", TOKEN_LOOP},
    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"ret", TOKEN_RETURN},
    {"match", TOKEN_MATCH},
    {"type", TOKEN_TYPE},
    {"struct", TOKEN_STRUCT},
    {"enum", TOKEN_ENUM},
    {"import", TOKEN_IMPORT},
    {"mut", TOKEN_MUT},
};

const struct { const char* symbol; TokenType_T type; } symbols[] = {
    {"++", TOKEN_INC},
    {"+=", TOKEN_ADD},
    {"+", TOKEN_PLUS},
    {"--", TOKEN_DEC},
    {"-=", TOKEN_SUB},
    {"-", TOKEN_MINUS},
    {"*=", TOKEN_MULT},
    {"*", TOKEN_STAR},
    {"/=", TOKEN_DIV},
    {"/", TOKEN_SLASH},
    {"&&", TOKEN_AND},
    {"&", TOKEN_REF},
    {"||", TOKEN_OR},
    {"==", TOKEN_EQ},
    {"=>", TOKEN_ARROW},
    {"=", TOKEN_ASSIGN},
    {"!=", TOKEN_NOT_EQ},
    {"!", TOKEN_BANG},
    {">=", TOKEN_GT_EQ},
    {">", TOKEN_GT},
    {"<=", TOKEN_LT_EQ},
    {"<-", TOKEN_RETURN},
    {"<", TOKEN_LT},
    {"(", TOKEN_LPAREN},
    {")", TOKEN_RPAREN},
    {"{", TOKEN_LBRACE},
    {"}", TOKEN_RBRACE},
    {"[", TOKEN_LBRACKET},
    {"]", TOKEN_RBRACKET},
    {"~", TOKEN_TILDE},
    {",", TOKEN_COMMA},
    {";", TOKEN_SEMICOLON},
    {"_", TOKEN_UNDERSCORE},
    {":", TOKEN_COLON},
    {"..", TOKEN_RANGE},
    {".", TOKEN_DOT},
    {NULL, TOKEN_EOF}   // the last one has to be null as an indicator for the end of the array
};  

static void lexer_skip_whitespace(Lexer_T* lexer);
static void lexer_skip_comment(Lexer_T* lexer);
static Token_T* lexer_get_id(Lexer_T* lexer);
static Token_T* lexer_get_number(Lexer_T* lexer);
static Token_T* lexer_get_symbol(Lexer_T* lexer);

Lexer_T* init_lexer(SrcFile_T* src) 
{
    Lexer_T* lexer = calloc(1, sizeof(struct LEXER_STRUCT));

    lexer->file = src;

    lexer->pos = 0;
    lexer->line = 0;
    lexer->c = get_char(lexer->file, lexer->line, lexer->pos);

    return lexer;
}

void free_lexer(Lexer_T* lexer)
{
    free(lexer);
}

void lexer_advance(Lexer_T* lexer)
{
    lexer->pos++;
    if(lexer->pos >= get_line_len(lexer->file, lexer->line))
    {
        if(lexer->line >= lexer->file->num_lines - 1)
        {
            lexer->c = '\0';
            // end of file
            return;
        }

        lexer->pos = 0;
        lexer->line++;
    }

    lexer->c = get_char(lexer->file, lexer->line, lexer->pos);
}

char lexer_peek(Lexer_T* lexer, int offset)
{
    if(lexer->pos + offset >= get_line_len(lexer->file, lexer->line))
        return -1;
    
    return get_char(lexer->file, lexer->line, lexer->pos + offset);
}

Token_T* lexer_consume(Lexer_T* lexer, Token_T* token)
{
    lexer_advance(lexer);
    return token;
}

Token_T* lexer_consume_type(Lexer_T* lexer, TokenType_T type)
{
    return lexer_consume(lexer, init_token((char[]){lexer->c, '\0'}, lexer->line, lexer->pos, type, lexer->file));
}

Token_T* lexer_next_token(Lexer_T* lexer)
{
    lexer_skip_whitespace(lexer);

    if(lexer->c == '#')
        lexer_skip_comment(lexer);

    if(isalpha(lexer->c))
        return lexer_get_id(lexer);
    else if(isdigit(lexer->c))
        return lexer_get_number(lexer);
    else 
        return lexer_get_symbol(lexer);
}

static void lexer_skip_whitespace(Lexer_T* lexer)
{
    while(lexer->c == '\t' || lexer->c == ' ' || lexer->c == '\r' || lexer->c == '\n')
    {
        lexer_advance(lexer);
    }
}

static void lexer_skip_multiline_comment(Lexer_T* lexer)
{
    unsigned int start_line = lexer->line;
    unsigned int start_pos = lexer->pos;

    lexer_advance(lexer);
    lexer_advance(lexer);

    while(lexer->c != '#' && lexer_peek(lexer, 1) != '#')
    {
        if(lexer->c == '\0')
        {    //end of file
            throw_error(ERR_SYNTAX_ERROR,  &(Token_T){.line = start_line, .pos = start_pos, .source = lexer->file}, "unterminated multiline comment");
            return;
        }
        lexer_advance(lexer);
    }

    lexer_advance(lexer);
    lexer_advance(lexer);
}

static void lexer_skip_comment(Lexer_T* lexer)
{
    if(lexer_peek(lexer, 1) == '#') {
        lexer_skip_multiline_comment(lexer);
    }
    else 
    {
        if(lexer->line >= lexer->file->num_lines - 1)
        {
            lexer->c = '\0';
            // end of file
            return;
        }

        lexer->pos = 0;
        lexer->line++;

        lexer->c = get_char(lexer->file, lexer->line, lexer->pos);
    }

    lexer_skip_whitespace(lexer);

    if(lexer->c == '#')
        lexer_skip_comment(lexer);
}

static TokenType_T lexer_get_id_type(char* id)
{
    TokenType_T type = TOKEN_ID;
    for(int i = 0; i < NUM_KEYWORDS; i++)
        if(strcmp(keywords[i].str, id) == 0)
            type = keywords[i].type;

    return type;
}

static Token_T* lexer_get_id(Lexer_T* lexer)
{
    char* buffer = calloc(1, sizeof(char));

    while(isalnum(lexer->c) || lexer->c == '_')
    {
        buffer = realloc(buffer, (strlen(buffer) + 2) * sizeof(char));
        strcat(buffer, (char[]){lexer->c, '\0'});
        lexer_advance(lexer);
    }

    Token_T* token = init_token(buffer, lexer->line, lexer->pos - 1, lexer_get_id_type(buffer), lexer->file);

    free(buffer);
    return token;
}

static Token_T* lexer_get_hexadecimal(Lexer_T* lexer)
{
    lexer_advance(lexer);    // cut the '0x'
    lexer_advance(lexer);

    char* buffer = calloc(1, sizeof(char));

    while(isxdigit(lexer->c) || lexer->c == '_')
    {
        if(lexer->c == '_')
        {
            lexer_advance(lexer);
            continue;
        }

        buffer = realloc(buffer, (strlen(buffer) + 2) * sizeof(char));
        strcat(buffer, (char[]){lexer->c, '\0'});

        lexer_advance(lexer);
    }

    long decimal = strtol(buffer, NULL, 16);
    buffer = realloc(buffer, (strlen("%ld") + 1) * sizeof(char));
    sprintf(buffer, "%ld", decimal);

    Token_T* token = init_token(buffer, lexer->line, lexer->pos, TOKEN_INT, lexer->file);
    free(buffer);
    return token;
}

static Token_T* lexer_get_binary(Lexer_T* lexer)
{
    lexer_advance(lexer);    // cut the '0b'
    lexer_advance(lexer);
    
    char* buffer = calloc(1, sizeof(char));

    while(lexer->c == '0' || lexer->c == '1' || lexer->c == '_')
    {
        if(lexer->c == '_')
        {
            lexer_advance(lexer);
            continue;
        }

        buffer = realloc(buffer, (strlen(buffer) + 2) * sizeof(char));
        strcat(buffer, (char[]){lexer->c, '\0'});

        lexer_advance(lexer);
    }

    long decimal = strtol(buffer, NULL, 2);
    buffer = realloc(buffer, (strlen("%ld") + 1) * sizeof(char));
    sprintf(buffer, "%ld", decimal);

    Token_T* token = init_token(buffer, lexer->line, lexer->pos, TOKEN_INT, lexer->file);
    free(buffer);
    return token;
}

static Token_T* lexer_get_decimal(Lexer_T* lexer)
{
    char* buffer = calloc(1, sizeof(char));
    TokenType_T type = TOKEN_INT;

    while(isdigit(lexer->c) || lexer->c == '.' || lexer->c == '_')
    {
        if(lexer->c == '_')
        {
            lexer_advance(lexer);
            continue;
        }

        buffer = realloc(buffer, (strlen(buffer) + 2) * sizeof(char));
        strcat(buffer, (char[]){lexer->c, '\0'});

        if(lexer->c == '.')
        {   
            if(lexer_peek(lexer, 1) == '.')
            {
                Token_T* token = init_token(buffer, lexer->line, lexer->pos, type, lexer->file);
                free(buffer);
                return token;
            }

            if(type == TOKEN_FLOAT)
            {
                Token_T* token = init_token(buffer, lexer->line, lexer->pos, type, lexer->file);

                free(buffer);
                
                throw_error(ERR_SYNTAX_ERROR,  &(Token_T){.line = lexer->line, .pos = lexer->pos, .source = lexer->file}, "multiple `.` found in number literal");
                return token;
            }

            type = TOKEN_FLOAT;
        }
        lexer_advance(lexer);
    }

    Token_T* token = init_token(buffer, lexer->line, lexer->pos, type, lexer->file);

    free(buffer);
    return token;
}

static Token_T* lexer_get_number(Lexer_T* lexer)
{
    if(lexer->c == '0')
    {
        switch(lexer_peek(lexer, 1)) {
            case 'x':
                return lexer_get_hexadecimal(lexer);
            case 'b':
                return lexer_get_binary(lexer);

            default:
                break;
        }
    }
    return lexer_get_decimal(lexer);    
}

static Token_T* lexer_get_str(Lexer_T* lexer)
{
    lexer_advance(lexer);

    char* buffer = calloc(1, sizeof(char));
    unsigned int start_line = lexer->line;
    unsigned int start_pos = lexer->pos;

    while(lexer->c != '"')
    {
        buffer = realloc(buffer, (strlen(buffer) + 2) * sizeof(char));
        strcat(buffer, (char[]){lexer->c, '\0'});
        lexer_advance(lexer);

        if(lexer->c == '\0')
        {
            free(buffer);
            throw_error(ERR_SYNTAX_ERROR,  &(Token_T){.line = start_line, .pos = start_pos, .source = lexer->file}, "unterminated string literal");
            return init_token("EOF", start_line, lexer->pos, TOKEN_EOF, lexer->file);
        }
    }
    lexer_advance(lexer);

    Token_T* token = init_token(buffer, lexer->line, lexer->pos, TOKEN_STRING, lexer->file);

    free(buffer);
    return token;
}

static Token_T* lexer_get_char(Lexer_T* lexer)
{
    lexer_advance(lexer);

    if(lexer->c == '\'')
    {
        throw_error(ERR_SYNTAX_ERROR,  &(Token_T){.line = lexer->line, .pos = lexer->pos, .source = lexer->file}, "empty char literal");
        return init_token("EOF", lexer->line, lexer->pos, TOKEN_EOF, lexer->file);
    }

    Token_T* token = init_token((char[]){lexer->c, 0}, lexer->line, lexer->pos, TOKEN_CHAR, lexer->file);
    lexer_advance(lexer);

    if(lexer->c != '\'')
    {
        throw_error(ERR_SYNTAX_ERROR,  &(Token_T){.line = lexer->line, .pos = lexer->pos, .source = lexer->file}, "unterminated char literal, expect `'`");
        free_token(token);

        return init_token("EOF", lexer->line, lexer->pos, TOKEN_EOF, lexer->file); 
    }
    lexer_advance(lexer);

    return token;
}

static Token_T* lexer_get_symbol(Lexer_T* lexer)
{
    for(int i = 0; symbols[i].symbol != NULL; i++)
    {
        const char* s = symbols[i].symbol;
        if(strlen(s) == 1 && lexer->c == s[0])
            return lexer_consume(lexer, init_token((char*) s, lexer->line, lexer->pos, symbols[i].type, lexer->file));
        if(strlen(s) == 2 && lexer->c == s[0] && lexer_peek(lexer, 1) == s[1])
            return lexer_consume(lexer, lexer_consume(lexer, init_token((char*) s, lexer->line, lexer->pos, symbols[i].type, lexer->file)));
    }

    switch(lexer->c) {

        case '"':
            return lexer_get_str(lexer);

        case '\'':
            return lexer_get_char(lexer);
        
        case '\0':
            return init_token("EOF", lexer->line, lexer->pos, TOKEN_EOF, lexer->file);

        default: {
            throw_error(ERR_SYNTAX_ERROR, &(Token_T){.line = lexer->line, .pos = lexer->pos, .source = lexer->file}, "unexpected symbol `%c` [id: %d]", lexer->c, lexer->c);
        }
    }
    // satisfy -Wall
    return NULL;
}