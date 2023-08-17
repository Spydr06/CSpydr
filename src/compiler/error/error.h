#ifndef CSPYDR_ERROR_HANDLER_H
#define CSPYDR_ERROR_HANDLER_H

#include <stdarg.h>
#include <stdbool.h>

#include "list.h"
#include "panic.h"
#include "lexer/token.h"

#define unreachable() \
    LOG_ERROR_F("internal error at %s:%d\n", __FILE__, __LINE__)

typedef enum ERROR_TYPE
{   
    ERR_SYNTAX_ERROR,   // syntax error
    ERR_SYNTAX_ERROR_UNCR, // uncritical syntax error
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
    ERR_CALL_ERROR,     // error while calling
    ERR_CALL_ERROR_UNCR,
    ERR_PKG_CONFIG,      // error when resolving library packages
    ERR_PKG_CONFIG_WARN, // library resolving warning
    ERR_OVERFLOW_UNCR,   // integer overflow detected
    ERR_DEPRECATED,      // object marked as deprecated encountered

    ERR_ILLEGAL_TYPE_CAST, // illegal cast from/to a type
    ERR_TYPE_CAST_WARN,    // warning when casting (implicit casts, etc.)

    ERR_TYPE_ERROR,      // error when converting/using a type
    ERR_TYPE_ERROR_UNCR, // same as ERR_TYPE_ERROR, but no exit(1) call
    ERR_CONST_ASSIGN,    // error when assigning a value to a constant variable

    ERR_INTERNAL, // internal error, user should never see this...
} ErrorType_T;

typedef void (*ErrorHandlerFn_T)(Context_T*, ErrorType_T, Token_T*, const char*, va_list, bool, const char*);

#ifdef __GNUC__
__attribute((format(printf, 4, 5)))
#endif
void throw_error(Context_T* context, ErrorType_T ty, Token_T* tok, const char* msg, ...);

void set_error_handler(ErrorHandlerFn_T fn);
ErrorHandlerFn_T get_error_handler();

void default_error_handler(Context_T* context, ErrorType_T ty, Token_T* tok, const char* format, va_list args, bool is_error, const char* error_str);

#endif