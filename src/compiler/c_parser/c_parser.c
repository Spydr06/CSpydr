#include "c_parser.h"
#include "ast/ast.h"
#include "config.h"
#include "error/error.h"
#include "io/log.h"
#include "lexer/token.h"

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
}

void c_parser_free(CParser_T* parser)
{
}

void parse_c_header(CParser_T* parser, ASTObj_T* surrounding_obj, Token_T* include_token, const char* header)
{
    parser->current_obj = surrounding_obj;
    if(!parser->context->flags.silent) {
        LOG_OK_F(COLOR_BOLD_BLUE "    Parsing " COLOR_RESET " %s", header);
        fflush(OUTPUT_STREAM);
    }

    throw_error(parser->context, ERR_INTERNAL, include_token, "C header includes are not implemented yet.");

    if(!parser->context->flags.silent)
        LOG_INFO("\33[2k\r");

    parser->current_obj = NULL;
}

