#include "error.h"
#include "ast/ast.h"
#include "io/log.h"
#include "globals.h"

#include <string.h>
#include <stdio.h>

static void print_current_fn(void) {
    static ASTObj_T* last_fn = NULL; // remember the last function to eliminate duplication in multiple errors of the same function
    if(global.current_fn && *global.current_fn && *global.current_fn != last_fn) 
    {
        char buf[BUFSIZ] = {};
        fprintf(ERR_OUTPUT_STREAM, COLOR_MAGENTA "In function " COLOR_BOLD_MAGENTA "%s()" COLOR_RESET COLOR_MAGENTA ":\n" COLOR_RESET, ast_id_to_str(buf, (*global.current_fn)->id, LEN(buf)));
        last_fn = *global.current_fn;
    }
    else
        last_fn = NULL;
}

void default_error_handler(ErrorType_T ty, Token_T* tok, const char* format, va_list args, bool is_error, const char* error_str)
{
    if(!tok)
    {
        LOG_ERROR(COLOR_BOLD_RED "[Error] " COLOR_RESET COLOR_RED);
        fprintf(ERR_OUTPUT_STREAM,"%s", format);
        LOG_ERROR("\n");
        return;
    }

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
    print_current_fn();

    fprintf(ERR_OUTPUT_STREAM, err_tmp1, source_file_path, (long) line, (long) character, is_error ? COLOR_BOLD_RED : COLOR_BOLD_YELLOW, error_str);
    vfprintf(ERR_OUTPUT_STREAM, format, args);
    fprintf(ERR_OUTPUT_STREAM, err_tmp2, ERR_LINE_NUMBER_SPACES, line, src_line, src_line[strlen(src_line) - 1] == '\n' ? "" : "\n ", 
            ERR_LINE_NUMBER_SPACES, "", (int) (character - strlen(tok->value)), "");

    for(u32 i = 0; i < strlen(tok->value) - 1; i++)
        fprintf(ERR_OUTPUT_STREAM, "~");
    fprintf(ERR_OUTPUT_STREAM, err_tmp3);
}

void default_panic_handler(void) 
{
    // Emit an error summary
    if(!global.silent)
    {
        if(global.emitted_errors && global.emitted_warnings)
        {
            LOG_ERROR_F(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " %u error%s and %u warning%s thrown during code validation; aborting.\n", global.emitted_errors, global.emitted_errors == 1 ? "" : "s", global.emitted_warnings, global.emitted_warnings == 1 ? "" : "s");
            exit(1);
        }
        else if(global.emitted_errors)
        {
            LOG_ERROR_F(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " %u error%s thrown during code validation; aborting.\n", global.emitted_errors, global.emitted_errors == 1 ? "" : "s");
            exit(1);
        }
        else if(global.emitted_warnings)
            LOG_WARN_F(COLOR_BOLD_YELLOW "[Warning]" COLOR_RESET COLOR_YELLOW " %u warning%s thrown during code validation\n", global.emitted_warnings, global.emitted_warnings == 1 ? "" : "s");
    }
}