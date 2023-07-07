#include "error.h"
#include "ast/ast.h"
#include "config.h"
#include "context.h"

#include <io/log.h>
#include <stdio.h>

static FILE* OUTPUT_FILE;

void linter_error_handler(Context_T* context, ErrorType_T ty, Token_T* tok, const char* format, va_list args, bool is_error, const char* error_str)
{
    char* source_file_path = tok->source->short_path ? tok->source->short_path : (char*) tok->source->path;

    fprintf(OUTPUT_FILE, "%s: %u\nfile: %s\nline: %u\ncol: %u\n", is_error ? "error" : "warning", is_error ? context->emitted_errors : context->emitted_warnings, source_file_path, tok->line + 1, tok->pos + 1);
    
    if(context->current_obj && *context->current_obj) 
    {
        char buf[BUFSIZ] = {};
        fprintf(OUTPUT_FILE,
            "%s: %s\n",
            obj_kind_to_str((*context->current_obj)->kind),
            ast_id_to_str(buf, (*context->current_obj)->id, LEN(buf))
        );
    }
    fprintf(OUTPUT_FILE, "desc: ");

    vfprintf(OUTPUT_FILE, format, args);
    fprintf(OUTPUT_FILE, "\n\n");

    fflush(OUTPUT_FILE);
}

void summary(Context_T* context)
{
    fprintf(OUTPUT_FILE, "summary:\nerrors: %u\nwarnings: %u\n", context->emitted_errors, context->emitted_warnings);
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