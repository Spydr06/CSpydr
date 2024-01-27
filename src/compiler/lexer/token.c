#include "token.h"
#include "memory/allocator.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SIZEOF_TOKEN(value) (sizeof(struct CSPYDR_TOKEN_STRUCT) + (strlen((value)) + 1) * sizeof(char))

Token_T* init_token(Allocator_T* alloc, char* value, u32 line, u32 pos, TokenType_T type, File_T* source)
{
    Token_T* token = allocator_malloc(alloc, SIZEOF_TOKEN(value));

    token->line = line;
    token->pos = pos;
    token->type = type;
    token->source = source;

    strcpy(token->value, value);

    return token;
}

char* token_to_str(Token_T* token)
{
    const char* template = "Tok: [type: %d, value: `%s`, line: %d, pos: %d]";
    char* buffer = calloc(strlen(template) + strlen(token->value) + 1, sizeof(char));

    sprintf(buffer, template, token->type, token->value, token->line, token->pos);

    return buffer;
}

Token_T* duplicate_token(Allocator_T* alloc, const Token_T* tok)
{
    Token_T* duplicate = allocator_malloc(alloc, SIZEOF_TOKEN(tok->value));
    memcpy(duplicate, tok, SIZEOF_TOKEN(tok->value));
    return duplicate;
}
