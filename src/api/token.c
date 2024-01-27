#include "api.h"
#include "include/cspydr.h"
#include "lexer/token.h"

CSpydrToken_T* csp_new_token(CSpydrAllocator_T* alloc, CSpydrTokenType_T type, uint32_t line, uint32_t pos, char value[])
{
    return init_token(alloc, value, line, pos, type, NULL);
}

CSpydrTokenType_T csp_token_get_type(CSpydrToken_T* tok)
{
    return tok ? tok->type : TOKEN_ERROR;
}

u32 csp_token_get_line(CSpydrToken_T* tok)
{
    return tok ? tok->line : 0;
}

u32 csp_token_get_position(CSpydrToken_T* tok)
{
    return tok ? tok->pos : 0;
}

char* csp_token_get_value(CSpydrToken_T* tok)
{
    return tok ? tok->value : "(null)";
}

char* csp_token_get_file(CSpydrToken_T* tok)
{
    return tok ? 
        tok->source ? 
            tok->source->path ? 
                tok->source->path : tok->source->short_path 
            : "" 
        : "";
}
