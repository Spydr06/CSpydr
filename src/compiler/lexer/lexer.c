#include "lexer.h"
#include "ast/ast.h"
#include "config.h"
#include "error/error.h"
#include "list.h"
#include "mem/mem.h"
#include "token.h"

#include "io/log.h"
#include "util.h"
#include "timer/timer.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const struct { 
    const char* str; 
    TokenType_T type; 
} keywords[] = {
    {"alignof", TOKEN_ALIGNOF},
    {"asm", TOKEN_ASM},
    {"break", TOKEN_BREAK},
    {"const", TOKEN_CONST},
    {"continue", TOKEN_CONTINUE},
    {"defer", TOKEN_DEFER},
    {"do", TOKEN_DO},
    {"else", TOKEN_ELSE},
    {"embed", TOKEN_EMBED},
    {"enum", TOKEN_ENUM},
    {"extern", TOKEN_EXTERN},
    {"false", TOKEN_FALSE},
    {"fn", TOKEN_FN},
    {"for", TOKEN_FOR},
    {"if", TOKEN_IF},
    {"import", TOKEN_IMPORT},
    {"interface", TOKEN_INTERFACE},
    {"len", TOKEN_LEN},
    {"let", TOKEN_LET},
    {"loop", TOKEN_LOOP},
    {"macro", TOKEN_MACRO},
    {"match", TOKEN_MATCH},
    {"namespace", TOKEN_NAMESPACE},
    {"nil", TOKEN_NIL},
    {"noop", TOKEN_NOOP},
    {"ret", TOKEN_RETURN},
    {"sizeof", TOKEN_SIZEOF},
    {"struct", TOKEN_STRUCT},
    {"true", TOKEN_TRUE},
    {"type", TOKEN_TYPE},
    {"typeof", TOKEN_TYPEOF},
    {"union", TOKEN_UNION},
    {"unless", TOKEN_UNLESS},
    {"using", TOKEN_USING},
    {"while", TOKEN_WHILE},
    {"with", TOKEN_WITH},
    {NULL, TOKEN_EOF}   // end of array indicator
};

const struct { 
    const char* symbol; 
    TokenType_T type;
} symbols[] = {
    {"(", TOKEN_LPAREN},
    {")", TOKEN_RPAREN},
    {"{", TOKEN_LBRACE},
    {"}", TOKEN_RBRACE},
    {"[", TOKEN_LBRACKET},
    {"]", TOKEN_RBRACKET},
    {",", TOKEN_COMMA},
    {";", TOKEN_SEMICOLON},
    {";", TOKEN_SEMICOLON}, // greek question mark (;)
    {"_", TOKEN_UNDERSCORE},
    {"²", TOKEN_POW_2},
    {"³", TOKEN_POW_3},
    {"$", TOKEN_DOLLAR},
    {"@", TOKEN_AT},
    {"...", TOKEN_VA_LIST},
    {"<-", TOKEN_RETURN},
    {"=>", TOKEN_ARROW},
    {"`", TOKEN_INFIX_CALL},
    {NULL, TOKEN_EOF}   // the last one has to be null as an indicator for the end of the array
};  

static void lexer_skip_whitespace(Lexer_T* lexer);
static void lexer_skip_comment(Lexer_T* lexer);
static Token_T* lexer_get_id(Lexer_T* lexer);
static Token_T* lexer_get_number(Lexer_T* lexer);
static Token_T* lexer_get_symbol(Lexer_T* lexer);

void init_lexer(Lexer_T* lexer, Context_T* context, File_T* src) 
{
    lexer->context = context;
    lexer->file = src;

    lexer->pos = 0;
    lexer->line = 0;
    lexer->c = get_char(lexer->file, lexer->line, lexer->pos);
}

i32 lexer_pass(Context_T* context, ASTProg_T* ast)
{
    timer_start(context, "lexing main file");

    ast->tokens = init_list();

    Lexer_T lex;
    init_lexer(&lex, context, ast->files->items[0]);

    Token_T* tok;
    for(tok = lexer_next_token(&lex); tok->type != TOKEN_EOF; tok = lexer_next_token(&lex))
        list_push(ast->tokens, tok);

    // push EOF token
    list_push(ast->tokens, tok);
    
    timer_stop(context);
    return 0;
}

static void lexer_advance(Lexer_T* lexer)
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

static char lexer_peek(Lexer_T* lexer, int offset)
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

    if(isalpha(lexer->c) || (lexer->c == '_' && (lexer_peek(lexer, 1) == '_' || isalnum(lexer_peek(lexer, 1)))))
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
    u32 start_line = lexer->line;
    u32 start_pos = lexer->pos;

    u32 depth = 1;

    lexer_advance(lexer);
    lexer_advance(lexer);

    while(lexer->c != ']' || lexer_peek(lexer, 1) != '#' || depth != 1)
    {
        if(lexer->c == '#' && lexer_peek(lexer, 1) == '[')
            depth++;
        else if(lexer->c == ']' && lexer_peek(lexer, 1) == '#')
            depth--;
        else if(lexer->c == '\0')
        {   
            //end of file
            throw_error(lexer->context, ERR_SYNTAX_ERROR,  init_token("#[", start_line, start_pos + 1, TOKEN_ID, lexer->file), "unterminated multiline comment");
            return;
        }
        lexer_advance(lexer);
    }

    lexer_advance(lexer);
    lexer_advance(lexer);
}

static void lexer_skip_comment(Lexer_T* lexer)
{
    if(lexer_peek(lexer, 1) == '[') {
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
    char buffer[BUFSIZ] = {'\0'};

    while(isalnum(lexer->c) || lexer->c == '_' || lexer->c == '?' || lexer->c == '\'')
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

    Token_T* id_token = init_token(buffer, lexer->line, lexer->pos - 1, is_macro ? TOKEN_MACRO_CALL : lexer_get_id_type(buffer), lexer->file);

    if(str_starts_with(id_token->value, "__csp_"))
        throw_error(lexer->context, ERR_SYNTAX_WARNING, id_token, "Unsafe identifier name:\nidentifiers starting with `__csp_` may be used internally");

    return id_token;
}

static Token_T* lexer_get_int(Lexer_T* lexer, const char* digits, i32 base)
{
    lexer_advance(lexer);    // cut the '0x'
    lexer_advance(lexer);

    char buffer[BUFSIZ] = {'\0'};

    while(strchr(digits, lexer->c))
    {
        if(lexer->c == '_')
        {
            lexer_advance(lexer);
            continue;
        }
        strcat(buffer, (char[2]){lexer->c, '\0'});

        lexer_advance(lexer);
    }

    u64 decimal = strtoll(buffer, NULL, base);
    sprintf(buffer, "%lu", decimal);

    Token_T* token = init_token(buffer, lexer->line, lexer->pos, TOKEN_INT, lexer->file);
    return token;
}

static Token_T* lexer_get_decimal(Lexer_T* lexer)
{
    char buffer[BUFSIZ] = {'\0'};

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
                throw_error(lexer->context, ERR_SYNTAX_ERROR,  &(Token_T){.line = lexer->line, .pos = lexer->pos, .source = lexer->file}, "multiple `.` found in number literal");

            type = TOKEN_FLOAT;
        }
        lexer_advance(lexer);
    }

    Token_T* token = init_token(buffer, lexer->line, lexer->pos - 1, type, lexer->file);
    return token;
}

static Token_T* lexer_get_number(Lexer_T* lexer)
{
    if(lexer->c == '0')
    {
        switch(lexer_peek(lexer, 1)) {
            case 'x':
                return lexer_get_int(lexer, "0123456789aAbBcCdDeEfF_", 16);
            case 'b':
                return lexer_get_int(lexer, "01_", 2);
            case 'o':
                return lexer_get_int(lexer, "01234567_", 8);

            default:
                break;
        }
    }
    return lexer_get_decimal(lexer); 
}

static Token_T* lexer_get_str(Lexer_T* lexer)
{
    u64 length = DEFAULT_STRING_TOKEN_SIZE;
    char* buffer = calloc(length, sizeof(char));

    size_t start_line = lexer->line;
    size_t start_pos = lexer->pos;

    lexer_advance(lexer);

    while(lexer->c != '"' || (lexer_peek(lexer, -1) == '\\' && lexer_peek(lexer, -2) != '\\'))
    {
        if(strlen(buffer) - 3 >= length)
            buffer = realloc(buffer, length *= 2);

        strcat(buffer, (char[2]){lexer->c, '\0'});

        lexer_advance(lexer);

        if(lexer->c == '\0')
            throw_error(lexer->context, ERR_SYNTAX_ERROR, init_token("\"", start_line, start_pos, TOKEN_STRING, lexer->file), "unterminated string literal, expect `\"`");        
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
        throw_error(lexer->context, ERR_SYNTAX_ERROR,  &(Token_T){.line = lexer->line, .pos = lexer->pos, .source = lexer->file}, "empty char literal");
        return init_token("EOF", lexer->line, lexer->pos, TOKEN_EOF, lexer->file);
    }

    char data[3] = {lexer->c, '\0', '\0'};

    switch(lexer->c)
    {
    case 'c':
    case 'C':
        if(lexer_peek(lexer, 1) != '\'') 
        {
            Token_T* tok = init_token((char[]){'\'', lexer->c, '\0'}, lexer->line, lexer->pos, TOKEN_C_ARRAY, lexer->file);
            lexer_advance(lexer);
            return tok;
        }
        break;

    case '\\':
        lexer_advance(lexer);
        data[0] = '\\';
        data[1] = lexer->c;
        data[2] = '\0';
        break;

    default:
        break;
    }
    
    Token_T* token = init_token(data, lexer->line, lexer->pos, TOKEN_CHAR, lexer->file);
    lexer_advance(lexer);

    if(lexer->c != '\'')
    {
        throw_error(lexer->context, ERR_SYNTAX_ERROR, init_token("'", lexer->line, lexer->pos, TOKEN_CHAR, lexer->file), "unterminated char literal, expect `'`");
        return init_token("EOF", lexer->line, lexer->pos, TOKEN_EOF, lexer->file); 
    }
    lexer_advance(lexer);

    return token;
}

static bool is_operator_char(char c)
{
    switch(c)
    {
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '=':
        case '&':
        case '^':
        case '<':
        case '>':
        case '|':
        case '!':
        case '?':
        case '.':
        case ':':
        case '~':
            return true;
        default:
            return false;
    }
}

static Token_T* lexer_get_operator(Lexer_T* lexer)
{
    char buffer[BUFSIZ] = {'\0'};

    while(is_operator_char(lexer->c))
    {
        strcat(buffer, (char[]){lexer->c, '\0'});
        lexer_advance(lexer);
    }

    Token_T* operator = init_token(buffer, lexer->line, lexer->pos - 1, TOKEN_OPERATOR, lexer->file);
    return operator;
}

static Token_T* lexer_get_symbol(Lexer_T* lexer)
{
    for(i32 i = 0; symbols[i].symbol != NULL; i++)
    {
        const char* s = symbols[i].symbol;
        if(strlen(s) == 1 && lexer->c == s[0])
            return lexer_consume(lexer, 
                init_token((char*) s, lexer->line, lexer->pos, symbols[i].type, lexer->file)
            );
        if(strlen(s) == 2 && lexer->c == s[0] && lexer_peek(lexer, 1) == s[1])
            return lexer_consume(lexer, 
                lexer_consume(lexer, 
                    init_token((char*) s, lexer->line, lexer->pos + 1, symbols[i].type, lexer->file)
                )
            );
        if(strlen(s) == 3 && lexer->c == s[0] && lexer_peek(lexer, 1) == s[1] && lexer_peek(lexer, 2) == s[2])
            return lexer_consume(lexer, 
                lexer_consume(lexer, 
                    lexer_consume(lexer, 
                        init_token((char*) s, lexer->line, lexer->pos + 2, symbols[i].type, lexer->file)
                    )
                )
            );
    }

    if(is_operator_char(lexer->c))
        return lexer_get_operator(lexer);

    switch(lexer->c) {

        case '"':
            return lexer_get_str(lexer);

        case '\'':
            return lexer_get_char(lexer);
        
        case '\0':
            return init_token("EOF", lexer->line, lexer->pos, TOKEN_EOF, lexer->file);

        default: {
            if(!lexer->c || lexer->c == -1) 
            {
                // file is empty, return EOF
                return init_token("EOF", lexer->line, lexer->pos, TOKEN_EOF, lexer->file);
            }
            else
                throw_error(lexer->context, ERR_SYNTAX_ERROR, init_token(&lexer->c, lexer->line, lexer->pos, TOKEN_ERROR, lexer->file), "unknown token `%c` (id: %d)", lexer->c, lexer->c);
        }
    }
    // satisfy -Wall
    return NULL;
}

bool token_is_keyword(TokenType_T type)
{
    for(size_t i = 0; keywords[i].str; i++) {
        if(keywords[i].type == type)
            return true;
    }
    return false;
}
