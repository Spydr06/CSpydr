#include "toolchain.h"

#include <string.h>
#include <assert.h>

#include "io/file.h"
#include "passes.h"
#include "lexer/lexer.h"
#include "preprocessor/preprocessor.h"
#include "parser/parser.h"
#include "parser/validator.h"
#include "parser/typechecker.h"
#include "optimizer/optimizer.h"
#include "codegen/transpiler/c_codegen.h"
#include "codegen/asm/asm_codegen.h"
#include "ast/ast_json.h"
#include "io/log.h"
#include "io/io.h"

#ifdef CSPYDR_USE_LLVM
    #include "codegen/llvm/llvm_codegen.h"
#endif

static i32 construct_passes(Pass_T passes[])
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
    push_pass(typechecker_pass);

    if(global.optimize)
        push_pass(optimizer_pass);
    
    switch(global.ct)
    {
        case CT_TRANSPILE:
            push_pass(transpiler_pass);
            break;
        case CT_ASM:
            push_pass(asm_codegen_pass);
            break;
#ifdef CSPYDR_USE_LLVM
        case CT_LLVM:
            push_pass(llvm_codegen_pass);
            break;
#endif
        case CT_TO_JSON:
            push_pass(serializer_pass);
            break;
        default:
            LOG_ERROR_F(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " Unknown compile type %d!\n", global.ct);
            panic();
    }

    push_pass(cleanup_pass);

#undef push_pass

    return index;
}

void compile(char* input_file, char* output_file)
{
    global.read_main_file_on_init = true;

    try(global.main_error_exception)
    {
        ASTProg_T ast = {};
        global.main_src_file = input_file,  
        global.target = output_file;
        // TODO: init ast

        Pass_T passes[__CSP_MAX_PASSES] = {0};

        // construct passes
        i32 num_passes = construct_passes(passes);

        for(i32 i = 0; i < num_passes; i++)
        {
            //printf("pass `%s` (%d/%d)\n", passes[i].desc, i, num_passes);
            i32 error = passes[i].func(&ast);
            if(error || global.emitted_errors)
                panic();
        }
    }
    catch {
        get_panic_handler()();
    }
}

i32 initialization_pass(ASTProg_T* ast)
{
    init_ast_prog(ast, global.main_src_file, global.target);
    ast->files = init_list();

    if(global.read_main_file_on_init)
    {
        File_T* main_file = read_file(ast->main_file_path);
        list_push(ast->files, main_file);
    }

    return 0;
}

i32 cleanup_pass(ASTProg_T *ast)
{
    for(size_t i = 0; i < ast->files->size; i++)
    {
        File_T* file = ast->files->items[i];
        free_file(file);
    }
    free_list(ast->files);

    return 0;
}