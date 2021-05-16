#include "token.h"

#include <stdlib.h>
#include <string.h>

#include <string.h>
#include <stdio.h>

token_T* initToken(char* value, unsigned int line, unsigned int pos, tokenType_T type)
{
    token_T* token = calloc(1, sizeof(token_T));

    token->line = line;
    token->pos = pos;
    token->type = type;

    token->value = strdup(value);

    return token;
}

void freeToken(token_T* token)
{
    free(token->value);
    free(token);
}

char* tokenToString(token_T* token)
{
    const char* template = "Tok: [type: %d, value: `%s`, line: %d, pos: %d]";
    char* buffer = calloc(strlen(template) + strlen(token->value) + 1, sizeof(char));

    sprintf(buffer, template, token->type, token->value, token->line, token->pos);

    return buffer;
}
