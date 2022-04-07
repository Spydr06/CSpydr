#include "linter.h"

#include <ast/ast.h>
#include <list.h>
#include <io/io.h>
#include <parser/parser.h>
#include <globals.h>
#include <mem/mem.h>

i32 lint(char* src_file)
{
    ASTProg_T ast = {};

    List_T* files = init_list();
    list_push(files, read_file(src_file));

    global.silent = true;
    parse(&ast, files, true);

    for(size_t i = 0; i < files->size; i++)
        free_file(files->items[i]);
    free_list(files);
    mem_free();

    return global.emitted_errors != 0 || global.emitted_warnings != 0;
}