#ifndef CSPYDR_ERROR_HANDLER_H
#define CSPYDR_ERROR_HANDLER_H

#include "../list.h"
#include <stdbool.h>

#include "../lexer/token.h"

#define unreachable() \
    LOG_ERROR_F("internal error at %s:%d\n", __FILE__, __LINE__)

typedef enum ERROR_TYPE
{   
    ERR_SYNTAX_ERROR,
    ERR_SYNTAX_WARNING,
    ERR_REDEFINITION,
    ERR_UNDEFINED,
    ERR_MISC,
    ERR_OUT_OF_SCOPE,
    ERR_UNUSED,
    ERR_CODEGEN,
    ERR_CODEGEN_WARN,

    ERR_ILLEGAL_TYPE_CAST,
    ERR_TYPE_CAST_WARN,

    ERR_TYPE_ERROR,
    ERR_CONST_ASSIGN,

    ERR_INTERNAL,
} ErrorType_T;

void throw_error(ErrorType_T ty, Token_T* tok, const char* msg, ...);

#endif