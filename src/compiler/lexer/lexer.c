#include "lexer.h"
#include "token.h"

#include "../io/log.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const struct { const char* str; TokenType_T type; } keywords[] = {
    {"true", TOKEN_TRUE},
    {"false", TOKEN_FALSE},
    {"nil", TOKEN_NIL},
    {"let", TOKEN_LET},
    {"fn", TOKEN_FN},
    {"loop", TOKEN_LOOP},
    {"while", TOKEN_WHILE},
    {"for", TOKEN_FOR},
    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"ret", TOKEN_RETURN},
    {"match", TOKEN_MATCH},
    {"type", TOKEN_TYPE},
    {"struct", TOKEN_STRUCT},
    {"enum", TOKEN_ENUM},
    {"import", TOKEN_IMPORT},
    {"const", TOKEN_CONST},
    {"extern", TOKEN_EXTERN},
    {"macro", TOKEN_MACRO},
    {"sizeof", TOKEN_SIZEOF},
    {"typeof", TOKEN_TYPEOF},
    {"break", TOKEN_BREAK},
    {"continue", TOKEN_CONTINUE},
    {"noop", TOKEN_NOOP},
    {NULL, TOKEN_EOF}   // end of array indicator
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
    {"|:", TOKEN_MACRO_BEGIN},
    {":|", TOKEN_MACRO_END},
    {"|", TOKEN_BIT_OR},
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
    for(int i = 0; keywords[i].str != NULL; i++)
        if(strcmp(keywords[i].str, id) == 0)
            type = keywords[i].type;

    return type;
}

static Token_T* lexer_get_id(Lexer_T* lexer)
{
    char buffer[__CSP_MAX_TOKEN_SIZE];
    strcpy(buffer, "");

    while(isalnum(lexer->c) || lexer->c == '_')
    {
        strcat(buffer, (char[]){lexer->c, '\0'});
        lexer_advance(lexer);
    }

    bool is_macro = false;
    if(lexer->c == '!')
    {
        lexer_advance(lexer);
        is_macro = true;
    }

    Token_T* token = init_token(buffer, lexer->line, lexer->pos - 1, is_macro ? TOKEN_MACRO_CALL : lexer_get_id_type(buffer), lexer->file);
    return token;
}

static Token_T* lexer_get_hexadecimal(Lexer_T* lexer)
{
    lexer_advance(lexer);    // cut the '0x'
    lexer_advance(lexer);

    char buffer[__CSP_MAX_TOKEN_SIZE];
    strcpy(buffer, "");

    while(isxdigit(lexer->c) || lexer->c == '_')
    {
        if(lexer->c == '_')
        {
            lexer_advance(lexer);
            continue;
        }
        strcat(buffer, (char[2]){lexer->c, '\0'});

        lexer_advance(lexer);
    }

    long decimal = strtol(buffer, NULL, 16);
    sprintf(buffer, "%ld", decimal);

    Token_T* token = init_token(buffer, lexer->line, lexer->pos, TOKEN_INT, lexer->file);
    return token;
}

static Token_T* lexer_get_binary(Lexer_T* lexer)
{
    lexer_advance(lexer);    // cut the '0b'
    lexer_advance(lexer);
    
    char buffer [__CSP_MAX_TOKEN_SIZE];
    strcpy(buffer, "");

    while(lexer->c == '0' || lexer->c == '1' || lexer->c == '_')
    {
        if(lexer->c == '_')
        {
            lexer_advance(lexer);
            continue;
        }

        strcat(buffer, (char[2]){lexer->c, '\0'});

        lexer_advance(lexer);
    }

    long decimal = strtol(buffer, NULL, 2);
    sprintf(buffer, "%ld", decimal);

    Token_T* token = init_token(buffer, lexer->line, lexer->pos, TOKEN_INT, lexer->file);
    return token;
}

static Token_T* lexer_get_decimal(Lexer_T* lexer)
{
    char buffer[__CSP_MAX_TOKEN_SIZE];
    strcpy(buffer, "");
    TokenType_T type = TOKEN_INT;

    while(isdigit(lexer->c) || lexer->c == '.' || lexer->c == '_')
    {
        if(lexer->c == '_')
        {
            lexer_advance(lexer);
            continue;
        }

        strcat(buffer, (char[2]){lexer->c, '\0'});

        if(lexer->c == '.')
        {   
            if(lexer_peek(lexer, 1) == '.')
            {
                Token_T* token = init_token(buffer, lexer->line, lexer->pos, type, lexer->file);
                return token;
            }

            if(type == TOKEN_FLOAT)
                throw_error(ERR_SYNTAX_ERROR,  &(Token_T){.line = lexer->line, .pos = lexer->pos, .source = lexer->file}, "multiple `.` found in number literal");

            type = TOKEN_FLOAT;
        }
        lexer_advance(lexer);
    }

    Token_T* token = init_token(buffer, lexer->line, lexer->pos, type, lexer->file);
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

    char buffer[__CSP_MAX_TOKEN_SIZE];
    strcpy(buffer, "");
    unsigned int start_line = lexer->line;
    unsigned int start_pos = lexer->pos;

    while(lexer->c != '"')
    {
        strcat(buffer, (char[2]){lexer->c, '\0'});
        lexer_advance(lexer);

        if(lexer->c == '\0')
        {
            throw_error(ERR_SYNTAX_ERROR,  &(Token_T){.line = start_line, .pos = start_pos, .source = lexer->file}, "unterminated string literal");
            return init_token("EOF", start_line, lexer->pos, TOKEN_EOF, lexer->file);
        }
    }
    lexer_advance(lexer);

    Token_T* token = init_token(buffer, lexer->line, lexer->pos, TOKEN_STRING, lexer->file);
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

    char data[3] = {lexer->c, '\0', '\0'};

    if(lexer->c == '\\')
    {
        lexer_advance(lexer);
        data[0] = '\\';
        data[1] = lexer->c;
        data[2] = '\0';
    }

    Token_T* token = init_token(data, lexer->line, lexer->pos, TOKEN_CHAR, lexer->file);
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