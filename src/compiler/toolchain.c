#include "toolchain.h"

#include "ast/mem/ast_mem.h"
#include "ast/ast.h"
#include "codegen/transpiler/c_codegen.h"
#include "codegen/llvm/llvm_codegen.h"
#include "parser/parser.h"
#include "io/file.h"
#include "io/log.h"
#include "io/io.h"
#include "codegen/asm/asm_codegen.h"

// generate the ast from the source file (lexing, preprocessing, parsing)
static void generate_ast(ASTProg_T* ast, char* path, char* target, bool silent);

// generate the output code (c, llvm, xml)
static void generate_llvm(ASTProg_T*, char* target, Action_T action, bool print_llvm, bool silent);
static void transpile_c(ASTProg_T*, char* target, Action_T action, bool print_c, bool silent);
static void parse_to_xml(ASTProg_T*, char* target, Action_T action, bool silent);
static void generate_asm(ASTProg_T* ast, char* target, Action_T action, bool print_asm, bool silent);

void compile(char* input_file, char* output_file, Action_T action)
{
    main_src_file = input_file;
    ASTProg_T ast = { 0 }; 
    generate_ast(&ast, input_file, output_file, silent);
    
    switch(ct)
    {
        case CT_LLVM:
            generate_llvm(&ast, output_file, action, print_llvm, silent);
            break;
        case CT_TRANSPILE:
            transpile_c(&ast, output_file, action, print_c, silent);
            break;
        case CT_TO_XML:
            parse_to_xml(&ast, output_file, action, silent);
            break;
        case CT_ASM:
            generate_asm(&ast, output_file, action, print_c, silent);
            break;
        default:
            LOG_ERROR_F("[Error] Unknown compile type %d!\n", ct);
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

// sets up and runs the compilation pipeline using LLVM
static void generate_llvm(ASTProg_T* ast, char* target, Action_T action, bool print_llvm, bool silent)
{
    LLVMCodegenData_T cg;
    init_llvm_cg(&cg, ast);
    cg.silent = silent;
    llvm_gen_code(&cg);

    if(print_llvm)
        llvm_print_code(&cg);

    switch(action)
    {
        case AC_BUILD:
            llvm_emit_code(&cg, target);
            break;
        case AC_RUN:
            llvm_run_code(&cg);
            break;
        case AC_DEBUG:
            // TODO:
            break;
        default:
            LOG_ERROR_F("Unrecognized action of type [%d], exit\n", action);
            exit(1);
    }
    free_llvm_cg(&cg);
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
    cg.print = print_c;

    asm_gen_code(&cg, target);
}

static void parse_to_xml(ASTProg_T* ast, char* target, Action_T action, bool silent)
{
    LOG_OK_F(COLOR_BOLD_GREEN "  Emitting " COLOR_RESET "  AST as XML to \"%s\"\n", target);
    //ast_to_xml(ast, target);
}
