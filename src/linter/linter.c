#include "linter.h"

#include <passes.h>

i32 lint(Context_T* context, char* src_file, char* std_path)
{
    ASTProg_T ast = {0};
    context->flags.read_main_file_on_init = true;
    context->paths.main_src_file = src_file;
    context->paths.std_path = std_path;
    context->paths.target = "/dev/null";

    initialization_pass(context, &ast);
    lexer_pass(context, &ast);
    preprocessor_pass(context, &ast);
    parser_pass(context, &ast);
    validator_pass(context, &ast);
    typechecker_pass(context, &ast);
    cleanup_pass(context, &ast);

    return context->emitted_errors != 0 || context->emitted_warnings != 0;
}