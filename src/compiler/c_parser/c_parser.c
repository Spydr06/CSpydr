#include "c_parser.h"

#include <stdio.h>
#include <string.h>

#include "libclang.h"

#include "ast/ast.h"
#include "config.h"
#include "context.h"
#include "error/error.h"
#include "io/log.h"
#include "lexer/token.h"
#include "list.h"
#include "clang-c/Index.h"

#define throw_error(...)              \
    do {                              \
        fprintf(OUTPUT_STREAM, "\n"); \
        throw_error(__VA_ARGS__);     \
    } while(0)

void c_parser_init(CParser_T* parser, Context_T* context, ASTProg_T* ast)
{
    parser->context = context;
    parser->ast = ast;
    parser->current_obj = NULL;
    parser->include_dirs = init_list();
}

void c_parser_free(CParser_T* parser)
{
    free_list(parser->include_dirs);
}

static enum CXChildVisitResult visit_children(CXCursor current_cursor, CXCursor parent, CXClientData client_data) {
    return CXChildVisit_Recurse;
}

static void create_dummy_file(struct CXUnsavedFile* file, const char* filename, const char* header) {
    file->Filename = filename; 
    file->Length = strlen(header) + 10;
    file->Contents = calloc(file->Length + 1, sizeof(char));
    snprintf((char*) file->Contents, file->Length, "#include<%s>", header);
}

static void free_dummy_file(struct CXUnsavedFile* file) {
    free((void*) file->Contents);
}

void parse_c_header(CParser_T* parser, ASTObj_T* surrounding_obj, Token_T* include_token, const char* header)
{
    parser->current_obj = surrounding_obj;
    if(!parser->context->flags.silent) {
        LOG_OK_F(COLOR_BOLD_BLUE "    Parsing " COLOR_RESET " %s", header);
        fflush(OUTPUT_STREAM);
    }

    struct CXUnsavedFile unsaved_files[1];
    create_dummy_file(&unsaved_files[0], "dummy.c", header); 

    CXIndex clang_index = clang_createIndex(false, true);
    CXTranslationUnit unit = clang_parseTranslationUnit(index, "dummy.c", NULL, 0, unsaved_files, LEN(unsaved_files), CXTranslationUnit_None);
    if(!unit) {
        throw_error(parser->context, ERR_C_PARSER, include_token, "error while parsing C file `%s`: ", header);
        goto finish;
    }

    CXCursor cursor = clang_getTranslationUnitCursor(unit);
    clang_visitChildren(cursor, visit_children, parser);

    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(clang_index);

    for(size_t i = 0; i < LEN(unsaved_files); i++)
        free_dummy_file(&unsaved_files[i]);

finish:
    if(!parser->context->flags.silent)
        LOG_INFO("\33[2k\r");

    throw_error(parser->context, ERR_INTERNAL, include_token, "C header includes are not implemented yet.");
    parser->current_obj = NULL;
}

void c_parser_add_include_dir(CParser_T* parser, const char* dir)
{
    char* owned_dir = strdup(dir);
    CONTEXT_ALLOC_REGISTER(parser->context, (void*) owned_dir);
    list_push(parser->include_dirs, (void*) owned_dir);
}

