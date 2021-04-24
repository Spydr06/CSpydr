#include "lexer.h"
#include "token.h"

#include "../io/log.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_KEYWORDS 13

struct {const char* str; tokenType_T type;} keyWords[NUM_KEYWORDS] = {
    {"true", TOKEN_TRUE},
    {"false", TOKEN_FALSE},
    {"nil", TOKEN_NIL},
    {"let", TOKEN_LET},
    {"fn", TOKEN_FN},
    {"loop", TOKEN_LOOP},
    {"if", TOKEN_IF},
    {"ret", TOKEN_RETURN},
    {"match", TOKEN_MATCH},
    {"type", TOKEN_TYPE},
    {"struct", TOKEN_STRUCT},
    {"enum", TOKEN_ENUM},
    {"import", TOKEN_IMPORT}
};

static void lexerSkipWhitespace(lexer_T* lexer);
static void lexerSkipComment(lexer_T* lexer);
static token_T* lexerGetId(lexer_T* lexer);
static token_T* lexerGetNumber(lexer_T* lexer);
static token_T* lexerGetSymbol(lexer_T* lexer);

lexer_T* initLexer(srcFile_T* src, errorHandler_T* eh) 
{
    lexer_T* lexer = calloc(1, sizeof(struct LEXER_STRUCT));

    lexer->file = src;
    lexer->eh = eh;

    lexer->pos = 0;
    lexer->line = 0;
    lexer->c = getChar(lexer->file, lexer->line, lexer->pos);

    return lexer;
}

void freeLexer(lexer_T* lexer)
{
    free(lexer);
}

void lexerAdvance(lexer_T* lexer)
{
    lexer->pos++;
    if(lexer->pos >= getLineLength(lexer->file, lexer->line))
    {
        if(lexer->line >= lexer->file->numLines - 1)
        {
            lexer->c = '\0';
            // end of file
            return;
        }

        lexer->pos = 0;
        lexer->line++;
    }

    lexer->c = getChar(lexer->file, lexer->line, lexer->pos);
}

char lexerPeek(lexer_T* lexer, int offset)
{
    if(lexer->pos + offset >= getLineLength(lexer->file, lexer->line))
        return -1;
    
    return getChar(lexer->file, lexer->line, lexer->pos + offset);
}

token_T* lexerConsume(lexer_T* lexer, token_T* token)
{
    lexerAdvance(lexer);
    return token;
}

token_T* lexerConsumeType(lexer_T* lexer, tokenType_T type)
{
    return lexerConsume(lexer, initToken((char[]){lexer->c, '\0'}, lexer->line, lexer->pos, type));
}

token_T* lexerNextToken(lexer_T* lexer)
{
    lexerSkipWhitespace(lexer);

    if(lexer->c == '#')
        lexerSkipComment(lexer);

    if(isalpha(lexer->c))
        return lexerGetId(lexer);
    else if(isdigit(lexer->c))
        return lexerGetNumber(lexer);
    else 
        return lexerGetSymbol(lexer);
}

static void lexerSkipWhitespace(lexer_T* lexer)
{
    while(lexer->c == '\t' || lexer->c == ' ' || lexer->c == '\r' || lexer->c == '\n')
    {
        lexerAdvance(lexer);
    }
}

static void lexerSkipMultilineComment(lexer_T* lexer)
{
    unsigned int startLine = lexer->line;
    unsigned int startPos = lexer->pos;

    lexerAdvance(lexer);
    lexerAdvance(lexer);

    while(lexer->c != '#' && lexerPeek(lexer, 1) != '#')
    {
        if(lexer->c == '\0')
        {    //end of file
            throwSyntaxError(lexer->eh, "unterminated multiline comment", startLine, startPos);
            return;
        }
        lexerAdvance(lexer);
    }

    lexerAdvance(lexer);
    lexerAdvance(lexer);
}

static void lexerSkipComment(lexer_T* lexer)
{
    if(lexerPeek(lexer, 1) == '#') {
        lexerSkipMultilineComment(lexer);
    }
    else 
    {
        if(lexer->line >= lexer->file->numLines - 1)
        {
            lexer->c = '\0';
            // end of file
            return;
        }

        lexer->pos = 0;
        lexer->line++;

        lexer->c = getChar(lexer->file, lexer->line, lexer->pos);
    }

    lexerSkipWhitespace(lexer);

    if(lexer->c == '#')
        lexerSkipComment(lexer);
}

static tokenType_T lexerGetIdType(char* id)
{
    tokenType_T type = TOKEN_ID;
    for(int i = 0; i < NUM_KEYWORDS; i++)
        if(strcmp(keyWords[i].str, id) == 0)
            type = keyWords[i].type;

    return type;
}

static token_T* lexerGetId(lexer_T* lexer)
{
    char* buffer = calloc(1, sizeof(char));

    while(isalnum(lexer->c) || lexer->c == '_')
    {
        buffer = realloc(buffer, (strlen(buffer) + 2) * sizeof(char));
        strcat(buffer, (char[]){lexer->c, '\0'});
        lexerAdvance(lexer);
    }

    token_T* token = initToken(buffer, lexer->line, lexer->pos, lexerGetIdType(buffer));

    free(buffer);
    return token;
}

static token_T* lexerGetNumber(lexer_T* lexer)
{
    char* buffer = calloc(1, sizeof(char));
    tokenType_T type = TOKEN_INT;

    while(isdigit(lexer->c) || lexer->c == '.')
    {
        buffer = realloc(buffer, (strlen(buffer) + 2) * sizeof(char));
        strcat(buffer, (char[]){lexer->c, '\0'});

        if(lexer->c == '.')
        {   
            if(lexerPeek(lexer, 1) == '.')
            {
                token_T* token = initToken(buffer, lexer->line, lexer->pos, type);
                free(buffer);
                return token;
            }

            if(type == TOKEN_FLOAT)
            {
                token_T* token = initToken(buffer, lexer->line, lexer->pos, type);

                free(buffer);
                
                throwSyntaxError(lexer->eh, "multiple `.` found in number", lexer->line, lexer->pos);
                return token;
            }

            type = TOKEN_FLOAT;
        }
        lexerAdvance(lexer);
    }

    token_T* token = initToken(buffer, lexer->line, lexer->pos, type);

    free(buffer);
    return token;
}

static token_T* lexerGetString(lexer_T* lexer)
{
    lexerAdvance(lexer);

    char* buffer = calloc(1, sizeof(char));
    unsigned int startLine = lexer->line;
    unsigned int startPos = lexer->pos;

    while(lexer->c != '"')
    {
        buffer = realloc(buffer, (strlen(buffer) + 2) * sizeof(char));
        strcat(buffer, (char[]){lexer->c, '\0'});
        lexerAdvance(lexer);

        if(lexer->c == '\0')
        {
            free(buffer);
            throwSyntaxError(lexer->eh, "unterminated string literal", startLine, startPos);
            return initToken("EOF", startLine, lexer->pos, TOKEN_EOF);
        }
    }
    lexerAdvance(lexer);

    token_T* token = initToken(buffer, lexer->line, lexer->pos, TOKEN_STRING);

    free(buffer);
    return token;
}

static token_T* lexerGetChar(lexer_T* lexer)
{
    lexerAdvance(lexer);

    if(lexer->c == '\'')
    {
        throwSyntaxError(lexer->eh, "empty char literal", lexer->line, lexer->pos);
        return initToken("EOF", lexer->line, lexer->pos, TOKEN_EOF);
    }

    token_T* token = initToken((char[]){lexer->c, 0}, lexer->line, lexer->pos, TOKEN_CHAR);
    lexerAdvance(lexer);

    if(lexer->c != '\'')
    {
        throwSyntaxError(lexer->eh, "unterminated char, expect `'`", lexer->line, lexer->pos);
        freeToken(token);

        return initToken("EOF", lexer->line, lexer->pos, TOKEN_EOF); 
    }
    lexerAdvance(lexer);

    return token;
}

static token_T* lexerGetSymbol(lexer_T* lexer)
{
    switch(lexer->c) {
        case '(':
            return lexerConsumeType(lexer, TOKEN_LPAREN);            
        case ')':
            return lexerConsumeType(lexer, TOKEN_RPAREN);      
        case '{':
            return lexerConsumeType(lexer, TOKEN_LBRACE);      
        case '}':
            return lexerConsumeType(lexer, TOKEN_RBRACE);      
        case '[':
            return lexerConsumeType(lexer, TOKEN_LBRACKET);      
        case ']':
            return lexerConsumeType(lexer, TOKEN_RBRACKET);    
        case ':':
            return lexerConsumeType(lexer, TOKEN_COLON);
        case ';': 
            return lexerConsumeType(lexer, TOKEN_SEMICOLON);
        case ',': 
            return lexerConsumeType(lexer, TOKEN_COMMA);
        case '_':
            return lexerConsumeType(lexer, TOKEN_UNDERSCORE);
        case '@': 
            return lexerConsumeType(lexer, TOKEN_AT);
        case '$':
            return lexerConsumeType(lexer, TOKEN_DOLLAR);

        case '&':
            return lexerConsume(lexer, lexerConsume(lexer, initToken("&&", lexer->line, lexer->pos, TOKEN_AND)));
        case '|':
            return lexerConsume(lexer, lexerConsume(lexer, initToken("||", lexer->line, lexer->pos, TOKEN_OR)));

        case '=':
            if(lexerPeek(lexer, 1) == '=') 
                return lexerConsume(lexer, lexerConsume(lexer, initToken("==", lexer->line, lexer->pos, TOKEN_EQ)));
            else if(lexerPeek(lexer, 1) == '>')
                return lexerConsume(lexer, lexerConsume(lexer, initToken("=>", lexer->line, lexer->pos, TOKEN_ARROW)));
            else
                return lexerConsume(lexer, initToken("=", lexer->line, lexer->pos, TOKEN_ASSIGN)); 

        case '.':
            if(lexerPeek(lexer, 1) == '.') 
                return lexerConsume(lexer, lexerConsume(lexer, initToken("..", lexer->line, lexer->pos, TOKEN_RANGE)));
            else
                return lexerConsume(lexer, initToken(".", lexer->line, lexer->pos, TOKEN_DOT)); 

        case '>': 
            if(lexerPeek(lexer, 1) == '=') 
                return lexerConsume(lexer, lexerConsume(lexer, initToken(">=", lexer->line, lexer->pos, TOKEN_GT_EQ)));
            else 
                return lexerConsume(lexer, initToken(">", lexer->line, lexer->pos, TOKEN_GT));

        case '<': 
            if(lexerPeek(lexer, 1) == '=') 
                return lexerConsume(lexer, lexerConsume(lexer, initToken("<=", lexer->line, lexer->pos, TOKEN_LT_EQ)));
            else 
                return lexerConsume(lexer, initToken("<", lexer->line, lexer->pos, TOKEN_LT));

        case '+': 
            if(lexerPeek(lexer, 1) == '=') 
                return lexerConsume(lexer, lexerConsume(lexer, initToken("+=", lexer->line, lexer->pos, TOKEN_ADD)));
            else if(lexerPeek(lexer, 1) == '+') 
                return lexerConsume(lexer, lexerConsume(lexer, initToken("++", lexer->line, lexer->pos, TOKEN_INC)));
            else 
                return lexerConsume(lexer, initToken("+", lexer->line, lexer->pos, TOKEN_PLUS));

        case '-': 
            if(lexerPeek(lexer, 1) == '=') 
                return lexerConsume(lexer, lexerConsume(lexer, initToken("-=", lexer->line, lexer->pos, TOKEN_SUB)));
            else if(lexerPeek(lexer, 1) == '-') 
                return lexerConsume(lexer, lexerConsume(lexer, initToken("--", lexer->line, lexer->pos, TOKEN_DEC)));
            else 
                return lexerConsume(lexer, initToken("-", lexer->line, lexer->pos, TOKEN_MINUS));

        case '*': 
            if(lexerPeek(lexer, 1) == '=') 
                return lexerConsume(lexer, lexerConsume(lexer, initToken("*=", lexer->line, lexer->pos, TOKEN_MULT)));
            else 
                return lexerConsume(lexer, initToken("*", lexer->line, lexer->pos, TOKEN_STAR));

        case '/': 
            if(lexerPeek(lexer, 1) == '=') 
                return lexerConsume(lexer, lexerConsume(lexer, initToken("/=",lexer->line, lexer->pos, TOKEN_DIV)));
            else 
                return lexerConsume(lexer, initToken("/", lexer->line, lexer->pos, TOKEN_SLASH));

        case '!': 
            if(lexerPeek(lexer, 1) == '=') 
                return lexerConsume(lexer, lexerConsume(lexer, initToken("!=", lexer->line, lexer->pos, TOKEN_NOT_EQ)));
            else 
                return lexerConsume(lexer, initToken("!", lexer->line, lexer->pos, TOKEN_BANG));

        case '"':
            return lexerGetString(lexer);

        case '\'':
            return lexerGetChar(lexer);

        case '\0':
            return initToken("EOF", lexer->line, lexer->pos, TOKEN_EOF);

        default: {
            const char* template = "unexpected symbol `%c`";
            char* msg = calloc(strlen(template) + 1, sizeof(char));
            sprintf(msg, template, lexer->c);

            throwSyntaxError(lexer->eh, msg, lexer->line, lexer->pos);
            return initToken("EOF", lexer->line, lexer->pos, TOKEN_EOF);
        }
    }
}