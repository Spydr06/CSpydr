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
        exit(1);
}

void set_error_handler(ErrorHandlerFn_T fn)
{
    ERROR_HANDLER = fn ? fn : default_error_handler;
}

ErrorHandlerFn_T get_error_handler()
{
    return ERROR_HANDLER;
}

void default_error_handler(ErrorType_T ty, Token_T* tok, const char* format, va_list args, bool is_error, const char* error_str)
{
    const char err_tmp1[] = COLOR_BOLD_WHITE "%s:%ld:%ld"    // file, line and character
                            COLOR_RESET " => %s[%s]"         // type of the error
                            COLOR_RESET ": ";                // before the error message
    const char err_tmp2[] = COLOR_RESET "\n"                 // after the error message
                            " %*d | %s %s"                   // the line number and source code line
                            "%*s | " COLOR_BOLD_BLUE "%*s^"; // the pointer to the error in the source
    const char err_tmp3[] = "-here" COLOR_RESET "\n";        // the end of the error message

    char* src_line = get_line(tok->source, tok->line);

    char* source_file_path = tok->source->short_path ? tok->source->short_path : (char*) tok->source->path;

    u32 line = tok->line + 1;
    u32 character = tok->pos + 1;

    // print the error
    if(global.current_fn && *global.current_fn) 
    {
        char buf[BUFSIZ] = {};
        fprintf(ERR_OUTPUT_STREAM, COLOR_MAGENTA "In function " COLOR_BOLD_MAGENTA "%s:\n" COLOR_RESET, ast_id_to_str(buf, (*global.current_fn)->id, LEN(buf)));
    }

    fprintf(ERR_OUTPUT_STREAM, err_tmp1, source_file_path, (long) line, (long) character, is_error ? COLOR_BOLD_RED : COLOR_BOLD_YELLOW, error_str);
    vfprintf(ERR_OUTPUT_STREAM, format, args);
    fprintf(ERR_OUTPUT_STREAM, err_tmp2, ERR_LINE_NUMBER_SPACES, line, src_line, src_line[strlen(src_line) - 1] == '\n' ? "" : "\n ", 
            ERR_LINE_NUMBER_SPACES, "", character - strlen(tok->value), "");

    for(u32 i = 0; i < strlen(tok->value) - 1; i++)
        fprintf(ERR_OUTPUT_STREAM, "~");
    fprintf(ERR_OUTPUT_STREAM, err_tmp3);
}
