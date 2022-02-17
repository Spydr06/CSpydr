#include "error.h"
#include "../io/log.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

static struct { const char* as_str; bool force_exit; bool is_error; } error_types[ERR_INTERNAL + 1] = {
    [ERR_SYNTAX_ERROR]      = {"syntax", true        , true},
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
    [ERR_MISC]              = {"misc", false         , true},       // miscellaneous errors, who cannot get categorized
    [ERR_CONST_ASSIGN]      = {"const assign", true  , true},
    [ERR_CODEGEN]           = {"codegen", true       , true},
    [ERR_CODEGEN_WARN]      = {"codegen warn", false , false},
    [ERR_CONSTEXPR]         = {"constexpr", true     , true},
    [ERR_CONSTEXPR_WARN]    = {"constexpr", false    , false},
    [ERR_NORETURN]          = {"no return", false    , true},
    [ERR_UNREACHABLE]       = {"unreachable", false  , false},
};

#ifdef __GNUC__
__attribute((format(printf, 3, 4)))
#endif
void throw_error(ErrorType_T ty, Token_T* tok, const char* format, ...)
{
    const char* err_tmp1 = COLOR_BOLD_WHITE "%s:%ld:%ld"           // file, line and character
                          COLOR_RESET " => %s[%s]"                 // type of the error
                          COLOR_RESET ": ";                        // before the error message
    const char* err_tmp2 = COLOR_RESET "\n"                        // after the error message
                          " %*d | %s %s"                           // the line number and source code line
                          "%*s | " COLOR_BOLD_BLUE "%*s^~here"     // the pointer to the error in the source
                          "\n" COLOR_RESET;                        // end of the message

    char* src_line = get_line(tok->source, tok->line);
    const char* err_ty_str = error_types[ty].as_str; 

    char* source_file_path = tok->source->short_path ? tok->source->short_path : (char*) tok->source->path;

    u32 line = tok->line + 1;
    u32 character = tok->pos + 1;

    va_list arg_list;
    va_start(arg_list, format);

    // print the error
    fprintf(ERR_OUTPUT_STREAM, err_tmp1, source_file_path, line, character, error_types[ty].is_error ? COLOR_BOLD_RED : COLOR_BOLD_YELLOW, err_ty_str);
    vfprintf(ERR_OUTPUT_STREAM, format, arg_list);
    fprintf(ERR_OUTPUT_STREAM, err_tmp2, ERR_LINE_NUMBER_SPACES, line, src_line, src_line[strlen(src_line) - 1] == '\n' ? "" : "\n ", ERR_LINE_NUMBER_SPACES, "", character - strlen(tok->value), "");

    va_end(arg_list);

    // exit if mandatory
    if(error_types[ty].force_exit)
        exit(1);
}
