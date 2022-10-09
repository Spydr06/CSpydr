#include "linter.h"

#include <globals.h>
#include <passes.h>

i32 lint(char* src_file, char* std_path)
{
    ASTProg_T ast = {0};
    global.read_main_file_on_init = true;
    global.main_src_file = src_file;
    global.std_path = std_path;
    global.target = "/dev/null";

    initialization_pass(&ast);
    lexer_pass(&ast);
    preprocessor_pass(&ast);
    parser_pass(&ast);
    validator_pass(&ast);
    typechecker_pass(&ast);
    cleanup_pass(&ast);

    return global.emitted_errors != 0 || global.emitted_warnings != 0;
}