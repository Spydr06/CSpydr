#include "lexer.h"
#include "errors/errorHandler.h"
#include "token.h"
#include "../log.h"
#include <string.h>
#include <ctype.h>

#define MAX(a, b) a > b ? a : b
#define MIN(a, b) a < b ? a : b

lexer_T* initLexer(char* src, char* path)
{
    lexer_T* lexer = calloc(1, sizeof(struct LEXER_STRUCT));
    lexer->src = src;
    lexer->srcSize = strlen(src);
    lexer->i = 0;
    lexer->line = 1;
    lexer->c = src[lexer->i];
    lexer->srcPath = path;
    lexer->iInLine = 0;
    lexer->currentLine = "";

    lexer->errorHandler = initErrorHandler();

    return lexer;
}

void lexerAdvance(lexer_T *lexer)
{
    if(lexer->i < lexer->srcSize && lexer->c != '\0')
    {
        lexer->i++;
        lexer->iInLine++;
        lexer->c = lexer->src[lexer->i];
    }
}

char lexerPeek(lexer_T* lexer, int offset)
{
    return lexer->src[MIN(lexer->i + offset, lexer->srcSize)];
}

token_T* lexerConsume(lexer_T* lexer, token_T* token)
{
    lexerAdvance(lexer);
    return token;
}

token_T* lexerConsumeType(lexer_T* lexer, tokenType_T type)
{
    char* value = calloc(2, sizeof(char));
    value[0] = lexer->c;
    value[1] = '\0';

    token_T* token = initToken(value, lexer->line, type);
    lexerAdvance(lexer);

    return token;
}

static void lexerSkipWhitespace(lexer_T* lexer)
{
    while(lexer->c == 13 || lexer->c == 10 || lexer->c == ' ' || lexer->c == '\t') 
    {
        if(lexer->c == '\n')
        {
            lexer->line++;
            lexer->iInLine = 0;

            char* line = calloc(1, sizeof(char));
            for(int i = 1; ; i++)
            {
                char currentChar = lexer->src[lexer->i + i];
                if(currentChar == '\n' || currentChar == '\0') {
                    break;
                }

                int len = strlen(line);
                line = realloc(line, (len + 2) * sizeof(char));

                line[len] = currentChar;
                line[len + 1] = '\0';
            }
            lexer->currentLine = line;

            pushSrcLine(lexer->errorHandler, line);
        }
        lexerAdvance(lexer);
    }
}

static token_T* lexerParseNumber(lexer_T* lexer)
{
    char* value = calloc(1, sizeof(char));

    while(isdigit(lexer->c) || lexer->c == '.') 
    {
        value = realloc(value, (strlen(value) + 2) * sizeof(char));
        strcat(value, (char[]){lexer->c, 0});
        lexerAdvance(lexer);
    }

    return initToken(value, lexer->line, TOKEN_NUMBER);
}

static token_T* lexerParseString(lexer_T* lexer)
{
    lexerAdvance(lexer);
    char* value = calloc(1, sizeof(char));
    
    while(lexer->c != '"')
    {
        value = realloc(value, (strlen(value) + 2) * sizeof(char));
        strcat(value, (char[]){lexer->c, 0});
        lexerAdvance(lexer);

        if(lexer->c == '\0') {
            LOG_ERROR("Unterminated string in line %d.\n", lexer->line);
            exit(1);
        }
    }
    lexerAdvance(lexer);

    return initToken(value, lexer->line, TOKEN_STR);
}

static token_T* lexerParseId(lexer_T* lexer)
{
    char* value = calloc(1, sizeof(char));
    tokenType_T type = TOKEN_ID;

    while(isalnum(lexer->c))
    {
        value = realloc(value, (strlen(value) + 2) * sizeof(char));
        strcat(value, (char[]){lexer->c, 0});
        lexerAdvance(lexer);
    }

    if(strcmp(value, "true") == 0 || strcmp(value, "false") == 0)
    {
        type = TOKEN_BOOL;
    }
    else if(strcmp(value, "i8") == 0 || strcmp(value, "i16") == 0 || strcmp(value, "i32") == 0 || strcmp(value, "i64") == 0 || 
            strcmp(value, "u8") == 0 || strcmp(value, "u16") == 0 || strcmp(value, "u32") == 0 || strcmp(value, "u64") == 0 ||
            strcmp(value, "f32") == 0 || strcmp(value, "f64") == 0 || 
            strcmp(value, "str") == 0 || strcmp(value, "bool") == 0 || strcmp(value, "char") == 0 || strcmp(value, "vec") == 0)
    {
        type = TOKEN_IDENTIFIER;
    }
    else if(strcmp(value, "let") == 0)
    {
        type = TOKEN_LET;
    }
    else if(strcmp(value, "fn") == 0)
    {
        type = TOKEN_FN;
    }
    else if(strcmp(value, "if") == 0 || strcmp(value, "else") == 0 || strcmp(value, "for") == 0 || strcmp(value, "while") == 0 || strcmp(value, "exit") == 0)
    {
        type = TOKEN_STMT;
    }
    else if(strcmp(value, "nil") == 0)
    {
        type = TOKEN_NIL;
    }

    return initToken(value, lexer->line, type);
}

static void lexerParseComment(lexer_T* lexer)
{
    while(lexer->c != '\n')
    {
        lexerAdvance(lexer);
    }
    lexerSkipWhitespace(lexer);

    if(lexer->c == '#')
    {
        lexerParseComment(lexer);
    }
}

static token_T* lexerParseSymbol(lexer_T* lexer)
{
    switch (lexer->c) {
        case '(': return lexerConsumeType(lexer, TOKEN_LEFT_PAREN);
        case ')': return lexerConsumeType(lexer, TOKEN_RIGHT_PAREN);
        case '{': return lexerConsumeType(lexer, TOKEN_LEFT_BRACE);
        case '}': return lexerConsumeType(lexer, TOKEN_RIGHT_BRACE);
        case '[': return lexerConsumeType(lexer, TOKEN_LEFT_BRACKET);
        case ']': return lexerConsumeType(lexer, TOKEN_RIGHT_BRACKET);
        case '=': {
            if(lexerPeek(lexer, 1) == '=') 
                return lexerConsume(lexer, lexerConsume(lexer, initToken("==", lexer->line, TOKEN_EQUALS_EQUALS)));
            else 
                return lexerConsume(lexer, initToken("=", lexer->line, TOKEN_EQUALS));
        }
        case ':': return lexerConsumeType(lexer, TOKEN_COLON);
        case ';': return lexerConsumeType(lexer, TOKEN_SEMICOLON);
        case '.': return lexerConsumeType(lexer, TOKEN_DOT);
        case ',': return lexerConsumeType(lexer, TOKEN_COMMA);
        case '>': {
            if(lexerPeek(lexer, 1) == '=') 
                return lexerConsume(lexer, lexerConsume(lexer, initToken(">=", lexer->line, TOKEN_GREATER_EQUALS)));
            else 
                return lexerConsume(lexer, initToken(">", lexer->line, TOKEN_GREATER));
        }
        case '<': {
            if(lexerPeek(lexer, 1) == '=') 
                return lexerConsume(lexer, lexerConsume(lexer, initToken("<=", lexer->line, TOKEN_LESS_EQUALS)));
            else if(lexerPeek(lexer, 1) == '-')
                return lexerConsume(lexer, lexerConsume(lexer, initToken("return", lexer->line, TOKEN_STMT)));
            else 
                return lexerConsume(lexer, initToken("<", lexer->line, TOKEN_LESS));
        }
        case '+': {
            if(lexerPeek(lexer, 1) == '=') 
                return lexerConsume(lexer, lexerConsume(lexer, initToken("+=", lexer->line, TOKEN_PLUS_EQUALS)));
            else 
                return lexerConsume(lexer, initToken("+", lexer->line, TOKEN_PLUS));
        }
        case '-': {
            if(lexerPeek(lexer, 1) == '=') 
                return lexerConsume(lexer, lexerConsume(lexer, initToken("-=", lexer->line, TOKEN_MINUS_EQUALS)));
            else 
                return lexerConsume(lexer, initToken("-", lexer->line, TOKEN_MINUS));
        }
        case '*': {
            if(lexerPeek(lexer, 1) == '=') 
                return lexerConsume(lexer, lexerConsume(lexer, initToken("*=", lexer->line, TOKEN_STAR_EQUALS)));
            else 
                return lexerConsume(lexer, initToken("*", lexer->line, TOKEN_STAR));
        }
        case '%': {
            if(lexerPeek(lexer, 1) == '=') 
                return lexerConsume(lexer, lexerConsume(lexer, initToken("%%=", lexer->line, TOKEN_PERCENT_EQUALS)));
            else 
                return lexerConsume(lexer, initToken("%%", lexer->line, TOKEN_PERCENT));
        }
        case '/': {
            if(lexerPeek(lexer, 1) == '=') 
                return lexerConsume(lexer, lexerConsume(lexer, initToken("/=",lexer->line, TOKEN_SLASH_EQUALS)));
            else 
                return lexerConsume(lexer, initToken("/", lexer->line, TOKEN_SLASH));
        }
        case '!': {
            if(lexerPeek(lexer, 1) == '=') 
                return lexerConsume(lexer, lexerConsume(lexer, initToken("!=", lexer->line, TOKEN_BANG_EQUALS)));
            else 
                return lexerConsume(lexer, initToken("!", lexer->line, TOKEN_BANG));
        }
        case '&':
            return lexerConsume(lexer, lexerConsume(lexer, initToken("&&", lexer->line, TOKEN_AND_AND)));
        case '|':
            return lexerConsume(lexer, lexerConsume(lexer, initToken("||", lexer->line, TOKEN_OR_OR)));
        case '_':
            return lexerConsumeType(lexer, TOKEN_UNDERSCORE);
        case '\"':
            return lexerParseString(lexer);
        case '\0': break; 
            return initToken(0, lexer->line, TOKEN_EOF);
        default: 
            LOG_ERROR("Unexpected character '%c' in line %d.", lexer->c, lexer->line);
            exit(1);
    }
    return initToken(0, lexer->line, TOKEN_EOF);
}

token_T* lexerNextToken(lexer_T* lexer)
{

    while(lexer->c != '\0') 
    {
        lexerSkipWhitespace(lexer);

        if(lexer->c == '#')
        {
            lexerParseComment(lexer);    
        }
        
        if(isalpha(lexer->c)) 
        {
            return lexerParseId(lexer);
        }
        else if(isdigit(lexer->c)) 
        {
            return lexerParseNumber(lexer);
        }
        else
        {
            return lexerParseSymbol(lexer);
        }
    }
    return initToken(0, lexer->line, TOKEN_EOF);
}