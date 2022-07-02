#include "error.h"
#include "io/log.h"
#include "ast/ast.h"
#include "config.h"
#include "globals.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

static struct { const char* as_str; bool force_exit; bool is_error; } error_types[ERR_INTERNAL + 1] = {
    [ERR_SYNTAX_ERROR]      = {"syntax", true        , true},
    [ERR_SYNTAX_ERROR_UNCR] = {"sytax", false        , true},
    [ERR_SYNTAX_WARNING]    = {"warning", false      , false},
    [ERR_UNDEFINED]         = {"undef", true         , true},
    [ERR_UNDEFINED_UNCR]    = {"undef", false        , true},
    [ERR_REDEFINITION]      = {"redef", true         , true},
    [ERR_REDEFINITION_UNCR] = {"redef", false        , true},
    [ERR_UNUSED]            = {"unused", false       , false},
    [ERR_ILLEGAL_TYPE_CAST] = {"illegal cast", true  , true},
    [ERR_TYPE_CAST_WARN]    = {"cast warning", false , false},
    [ERR_INTERNAL]          = {"internal", true      , true},
    [ERR_TYPE_ERROR]        = {"type", true          , true},
    [ERR_TYPE_ERROR_UNCR]   = {"type", false         , true},
    [ERR_MISC]              = {"misc", false         , true},
    [ERR_CONST_ASSIGN]      = {"const assign", true  , true},
    [ERR_CODEGEN]           = {"codegen", true       , true},
    [ERR_CODEGEN_WARN]      = {"codegen warn", false , false},
    [ERR_CONSTEXPR]         = {"constexpr", true     , true},
    [ERR_CONSTEXPR_WARN]    = {"constexpr", false    , false},
    [ERR_NORETURN]          = {"no return", false    , true},
    [ERR_UNREACHABLE]       = {"unreachable", false  , false},
    [ERR_CALL_ERROR]        = {"call", true          , true},
    [ERR_CALL_ERROR_UNCR]   = {"call", false         , true},
};

static ErrorHandlerFn_T ERROR_HANDLER = default_error_handler;

#ifdef __GNUC__
__attribute((format(printf, 3, 4)))
#endif
void throw_error(ErrorType_T ty, Token_T* tok, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    if(error_types[ty].is_error)
        global.emitted_errors++;
    else
        global.emitted_warnings++;

    ERROR_HANDLER(ty, tok, format, args, error_types[ty].is_error, error_types[ty].as_str);
    va_end(args);

    // exit if mandatory
    if(error_types[ty].force_exit)
        panic();
}

void set_error_handler(ErrorHandlerFn_T fn)
{
    ERROR_HANDLER = fn ? fn : default_error_handler;
}

ErrorHandlerFn_T get_error_handler()
{
    return ERROR_HANDLER;
}

