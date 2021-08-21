#include "token.h"

#include <stdlib.h>
#include <string.h>

#include <string.h>
#include <stdio.h>

Token_T* init_token(char* value, unsigned int line, unsigned int pos, TokenType_T type, SrcFile_T* source)
{
    Token_T* token = calloc(1, sizeof(struct TOKEN_STRUCT));

    token->line = line;
    token->pos = pos;
    token->type = type;

    strcpy(token->value, value);
    token->source = source;

    return token;
}

void free_token(Token_T* token)
{
    free(token);
}

char* token_to_str(Token_T* token)
{
    const char* template = "Tok: [type: %d, value: `%s`, line: %d, pos: %d]";
    char* buffer = calloc(strlen(template) + strlen(token->value) + 1, sizeof(char));

    sprintf(buffer, template, token->type, token->value, token->line, token->pos);

    return buffer;
}

Token_T* dupl_token(Token_T* tok)
{
    return init_token(tok->value, tok->line, tok->pos, tok->type, tok->source);
}