#include "error.h"
#include "../io/log.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define LINE_NUMBER_SPACES 4
#define OUTPUT_FILE_STREAM stderr

static struct { const char* as_str; bool force_exit; } error_types[ERR_INTERNAL + 1] = {
    [ERR_SYNTAX_ERROR]      = {"syntax", true},
    [ERR_SYNTAX_WARNING]    = {"warning", false},
    [ERR_UNDEFINED]         = {"undef", true},
    [ERR_REDEFINITION]      = {"redef", true},
    [ERR_UNUSED]            = {"unused", false},
    [ERR_ILLEGAL_TYPE_CAST] = {"illegal cast", true},
    [ERR_TYPE_CAST_WARN]    = {"cast warning", false},
    [ERR_INTERNAL]          = {"internal", true},
    [ERR_TYPE_ERROR]        = {"type", true},
    [ERR_MISC]              = {"misc", false},       // miscellaneous errors, who cannot get categorized
    [ERR_CONST_ASSIGN]      = {"const assign", true}
};

void throw_error(ErrorType_T ty, Token_T* tok, const char* format, ...)
{
    const char* err_tmp1 = COLOR_BOLD_WHITE "%s:%ld:%ld"           // file, line and character
                          COLOR_RESET " => " COLOR_BOLD_RED "[%s]" // type of the error
                          COLOR_RESET ": ";                        //before the error message
    const char* err_tmp2 = COLOR_RESET "\n"                        // after the error message
                          " %*d | %s %s"                             // the line number and source code line
                          "%*s | " COLOR_BOLD_BLUE "%*s^~here"     // the pointer to the error in the source
                          "\n" COLOR_RESET;                        // end of the message

    char* src_line = get_line(tok->source, tok->line);
    const char* err_ty_str = error_types[ty].as_str; 

    char* source_file_path = tok->source->short_path ? tok->source->short_path : (char*) tok->source->path;

    unsigned int line = tok->line + 1;
    unsigned int character = tok->pos + 1;

    va_list arg_list;
    va_start(arg_list, format);

    // print the error
    fprintf(OUTPUT_FILE_STREAM, err_tmp1, source_file_path, line, character, err_ty_str);
    vfprintf(OUTPUT_FILE_STREAM, format, arg_list);
    fprintf(OUTPUT_FILE_STREAM, err_tmp2, LINE_NUMBER_SPACES, line, src_line, src_line[strlen(src_line) - 1] == '\n' ? "" : "\n ", LINE_NUMBER_SPACES, "", character - strlen(tok->value), "");

    va_end(arg_list);

    // exit if mandatory
    if(error_types[ty].force_exit)
        exit(1);
}
