#include "error.h"
#include "ast/ast.h"
#include "config.h"

#include <io/log.h>
#include <globals.h>
#include <stdio.h>

static FILE* OUTPUT_FILE;

void linter_error_handler(ErrorType_T ty, Token_T* tok, const char* format, va_list args, bool is_error, const char* error_str)
{
    char* source_file_path = tok->source->short_path ? tok->source->short_path : (char*) tok->source->path;

    fprintf(OUTPUT_FILE, "%s: %u\nfile: %s\nline: %u\ncol: %u\n", is_error ? "error" : "warning", is_error ? global.emitted_errors : global.emitted_warnings, source_file_path, tok->line + 1, tok->pos + 1);
    
    if(global.current_obj && *global.current_obj) 
    {
        char buf[BUFSIZ] = {};
        fprintf(OUTPUT_FILE,
            "%s: %s\n",
            obj_kind_to_str((*global.current_obj)->kind),
            ast_id_to_str(buf, (*global.current_obj)->id, LEN(buf))
        );
    }
    fprintf(OUTPUT_FILE, "desc: ");

    vfprintf(OUTPUT_FILE, format, args);
    fprintf(OUTPUT_FILE, "\n\n");

    fflush(OUTPUT_FILE);
}

void summary()
{
    fprintf(OUTPUT_FILE, "summary:\nerrors: %u\nwarnings: %u\n", global.emitted_errors, global.emitted_warnings);
}

void set_error_output_file(FILE* fp)
{
    OUTPUT_FILE = fp;
}

void close_output_file()
{
    if(OUTPUT_FILE == stderr || OUTPUT_FILE == stdout)
        return;
    
    fclose(OUTPUT_FILE);
}