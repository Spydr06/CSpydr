#include "lexer.h"
#include "token.h"

#include "../io/log.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_KEYWORDS 15

const struct { const char* str; tokenType_T type; } keyWords[NUM_KEYWORDS] = {
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

const struct { const char* symbol; tokenType_T type; } symbols[] = {
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

static token_T* lexerGetHexadecimal(lexer_T* lexer)
{
    lexerAdvance(lexer);    // cut the '0x'
    lexerAdvance(lexer);

    char* buffer = calloc(1, sizeof(char));

    while(isxdigit(lexer->c) || lexer->c == '_')
    {
        if(lexer->c == '_')
        {
            lexerAdvance(lexer);
            continue;
        }

        buffer = realloc(buffer, (strlen(buffer) + 2) * sizeof(char));
        strcat(buffer, (char[]){lexer->c, '\0'});

        lexerAdvance(lexer);
    }

    long decimal = strtol(buffer, NULL, 16);
    buffer = realloc(buffer, (strlen("%ld") + 1) * sizeof(char));
    sprintf(buffer, "%ld", decimal);

    token_T* token = initToken(buffer, lexer->line, lexer->pos, TOKEN_INT);
    free(buffer);
    return token;
}

static token_T* lexerGetBinary(lexer_T* lexer)
{
    lexerAdvance(lexer);    // cut the '0b'
    lexerAdvance(lexer);
    
    char* buffer = calloc(1, sizeof(char));

    while(lexer->c == '0' || lexer->c == '1' || lexer->c == '_')
    {
        if(lexer->c == '_')
        {
            lexerAdvance(lexer);
            continue;
        }

        buffer = realloc(buffer, (strlen(buffer) + 2) * sizeof(char));
        strcat(buffer, (char[]){lexer->c, '\0'});

        lexerAdvance(lexer);
    }

    long decimal = strtol(buffer, NULL, 2);
    buffer = realloc(buffer, (strlen("%ld") + 1) * sizeof(char));
    sprintf(buffer, "%ld", decimal);

    token_T* token = initToken(buffer, lexer->line, lexer->pos, TOKEN_INT);
    free(buffer);
    return token;
}

static token_T* lexerGetDecimal(lexer_T* lexer)
{
    char* buffer = calloc(1, sizeof(char));
    tokenType_T type = TOKEN_INT;

    while(isdigit(lexer->c) || lexer->c == '.' || lexer->c == '_')
    {
        if(lexer->c == '_')
        {
            lexerAdvance(lexer);
            continue;
        }

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

static token_T* lexerGetNumber(lexer_T* lexer)
{
    if(lexer->c == '0')
    {
        switch(lexerPeek(lexer, 1)) {
            case 'x':
                return lexerGetHexadecimal(lexer);
            case 'b':
                return lexerGetBinary(lexer);

            default:
                break;
        }
    }
    return lexerGetDecimal(lexer);    
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
    for(int i = 0; symbols[i].symbol != NULL; i++)
    {
        const char* s = symbols[i].symbol;
        if(strlen(s) == 1 && lexer->c == s[0])
            return lexerConsume(lexer, initToken((char*) s, lexer->line, lexer->pos, symbols[i].type));
        if(strlen(s) == 2 && lexer->c == s[0] && lexerPeek(lexer, 1) == s[1])
            return lexerConsume(lexer, lexerConsume(lexer, initToken((char*) s, lexer->line, lexer->pos, symbols[i].type)));
    }

    switch(lexer->c) {

        case '"':
            return lexerGetString(lexer);

        case '\'':
            return lexerGetChar(lexer);
        
        case '\0':
            return initToken("EOF", lexer->line, lexer->pos, TOKEN_EOF);

        default: {
            const char* template = "unexpected symbol `%c` [id: %d]";
            char* msg = calloc(strlen(template) + 1, sizeof(char));
            sprintf(msg, template, lexer->c, lexer->c);

            throwSyntaxError(lexer->eh, msg, lexer->line, lexer->pos);
            return initToken("EOF", lexer->line, lexer->pos, TOKEN_EOF);
        }
    }
}