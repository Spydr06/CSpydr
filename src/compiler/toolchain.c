#include "toolchain.h"

#include <assert.h>

#include "codegen/codegen.h"
#include "error/panic.h"
#include "io/file.h"
#include "ir/debug.h"
#include "ir/ir.h"
#include "ir/normalizer.h"
#include "passes.h"
#include "lexer/lexer.h"
#include "preprocessor/preprocessor.h"
#include "parser/parser.h"
#include "parser/validator.h"
#include "optimizer/optimizer.h"
#include "io/io.h"

static i32 construct_ast_passes(Context_T* context, Pass_T passes[])
{
    i32 index = 0;
#define push_pass(fn) do {                        \
        passes[index++] = (Pass_T){(#fn), (fn)};  \
        assert(index < __CSP_MAX_PASSES);         \
    } while(0)        

    push_pass(initialization_pass);
    push_pass(lexer_pass);
    push_pass(preprocessor_pass);
    push_pass(parser_pass);
    push_pass(validator_pass);

    if(context->flags.optimize)
        push_pass(optimizer_pass);

#undef push_pass

    return index;
}

void compile(Context_T* context, char* input_file, char* output_file)
{
    context->flags.read_main_file_on_init = true;

    try(context->main_error_exception)
    {
        ASTProg_T ast = {};
        context->paths.main_src_file = input_file,  
        context->paths.target = output_file;
        // TODO: init ast

        Pass_T passes[__CSP_MAX_PASSES] = {0};

        // construct passes
        i32 num_passes = construct_ast_passes(context, passes);

        for(i32 i = 0; i < num_passes; i++)
        {
            //printf("pass `%s` (%d/%d)\n", passes[i].desc, i, num_passes);
            i32 error = passes[i].func(context, &ast);
            if(error || context->emitted_errors)
                panic(context);
        }
        IR_T ir = {};
        i32 error = normalization_pass(context, &ast, &ir);
        if(error || context->emitted_errors)
            panic(context);

        dbg_print_ir(&ir, IR_PRINT_FUNC);

        error = codegen_pass(context, &ir);
        if(error || context->emitted_errors)
            panic(context);

        cleanup_pass(context, &ast);
    }
    catch {
        get_panic_handler()(context);
    }
}

i32 initialization_pass(Context_T* context, ASTProg_T* ast)
{
    init_ast_prog(context, ast, context->paths.main_src_file, context->paths.target);
    ast->files = init_list();

    if(context->flags.read_main_file_on_init)
    {
        File_T* main_file = read_file(ast->main_file_path);
        list_push(ast->files, main_file);
    }

    return 0;
}

i32 cleanup_pass(Context_T* _context, ASTProg_T *ast)
{
    for(size_t i = 0; i < ast->files->size; i++)
    {
        File_T* file = ast->files->items[i];
        free_file(file);
    }
    free_list(ast->files);

    return 0;
}
