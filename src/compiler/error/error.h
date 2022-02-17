#ifndef CSPYDR_ERROR_HANDLER_H
#define CSPYDR_ERROR_HANDLER_H

#include "../list.h"
#include <stdbool.h>

#include "../lexer/token.h"

#define unreachable() \
    LOG_ERROR_F("internal error at %s:%d\n", __FILE__, __LINE__)

typedef enum ERROR_TYPE
{   
    ERR_SYNTAX_ERROR,   // syntax error
    ERR_SYNTAX_WARNING, // syntax warning
    ERR_REDEFINITION,   // redefinition of identifier/type
    ERR_REDEFINITION_UNCR, // uncritical redefinition
    ERR_UNDEFINED,      // undefined error
    ERR_UNDEFINED_UNCR, // same as ERR_TYPE_ERROR, but no exit(1) call
    ERR_MISC,           // miscellaneous errors
    ERR_OUT_OF_SCOPE,   // variable or object out of scope
    ERR_UNUSED,         // unused variable or function
    ERR_CODEGEN,        // error during code generation
    ERR_CODEGEN_WARN,   // warning during code generation
    ERR_CONSTEXPR,      // error in a constant expression
    ERR_CONSTEXPR_WARN, // warning in a constant expression
    ERR_NORETURN,       // function does not return a value
    ERR_UNREACHABLE,    // "unreachable" code found after a return statement

    ERR_ILLEGAL_TYPE_CAST, // illegal cast from/to a type
    ERR_TYPE_CAST_WARN,    // warning when casting (implicit casts, etc.)

    ERR_TYPE_ERROR,      // error when converting/using a type
    ERR_TYPE_ERROR_UNCR, // same as ERR_TYPE_ERROR, but no exit(1) call
    ERR_CONST_ASSIGN,    // error when assigning a value to a constant variable

    ERR_INTERNAL, // internal error, user should never see this...
} ErrorType_T;

#ifdef __GNUC__
__attribute((format(printf, 3, 4)))
#endif
void throw_error(ErrorType_T ty, Token_T* tok, const char* msg, ...);

#endif