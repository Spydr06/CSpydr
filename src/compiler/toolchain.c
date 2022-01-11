#include "toolchain.h"

#include "ast/mem/ast_mem.h"
#include "ast/ast.h"
#include "codegen/transpiler/c_codegen.h"
#include "parser/parser.h"
#include "io/file.h"
#include "io/log.h"
#include "io/io.h"
#include "codegen/asm/asm_codegen.h"
#include "ast/ast_json.h"

// generate the ast from the source file (lexing, preprocessing, parsing)
static void generate_ast(ASTProg_T* ast, char* path, char* target, bool silent);

// generate the output code (c, llvm, xml)
static void generate_llvm(ASTProg_T*, char* target, Action_T action, bool print_llvm, bool silent);
static void transpile_c(ASTProg_T*, char* target, Action_T action, bool print_c, bool silent);
static void generate_asm(ASTProg_T* ast, char* target, Action_T action, bool print_asm, bool silent);
static void generate_json(ASTProg_T* ast, char* target, bool print_json, bool silent);

void compile(char* input_file, char* output_file, Action_T action)
{
    global.main_src_file = input_file;

    ASTProg_T ast = { 0 };
    if(global.from_json)
        ast_from_json(&ast, input_file);
    else
        generate_ast(&ast, input_file, output_file, global.silent);

    switch(global.ct)
    {
        case CT_TRANSPILE:
            transpile_c(&ast, output_file, action, global.print_code, global.silent);
            break;
        case CT_ASM:
            generate_asm(&ast, output_file, action, global.print_code, global.silent);
            break;
        case CT_TO_JSON:
            generate_json(&ast, output_file, global.print_code, global.silent);
            break;
        default:
            LOG_ERROR_F("[Error] Unknown compile type %d!\n", global.ct);
            exit(1);
    }

    for(size_t i = 0; i < ast.imports->size; i++)
        free_srcfile(ast.imports->items[i]);
    ast_free();
}



static void generate_ast(ASTProg_T* ast, char* path, char* target, bool silent)
{
    List_T* files = init_list(sizeof(struct SRC_FILE_STRUCT*));
    list_push(files, read_file(path));

    parse(ast, files, silent);
    //optimize(ast);
    
    ast->imports = files;
    ast_mem_add_list(files);
}

static void transpile_c(ASTProg_T* ast, char* target, Action_T action, bool print_c, bool silent)
{
    CCodegenData_T cg;
    init_c_cg(&cg, ast);
    cg.print_c = print_c;
    cg.silent = silent;
    c_gen_code(&cg, target);

    if(action == AC_RUN)
    {
        run_c_code(&cg, target);
        remove(target);
    }

    free(cg.buf);
}

static void generate_asm(ASTProg_T* ast, char* target, Action_T action, bool print_asm, bool silent)
{
    ASMCodegenData_T cg;
    init_asm_cg(&cg, ast);
    cg.silent = silent;
    cg.print = print_asm;

    asm_gen_code(&cg, target);

    if(action == AC_RUN)
    {
        asm_run_code(&cg, target);
        remove(target);
    }
    
    free(cg.buf);
}

static void generate_json(ASTProg_T* ast, char* target, bool print_json, bool silent)
{
    if(!silent)
        LOG_OK_F(COLOR_BOLD_BLUE "  Generating" COLOR_BOLD_WHITE " JSON" COLOR_RESET " to %s\n", target);
    ast_to_json(ast, target, print_json);
}