#include <llvm-c/BitWriter.h>
#ifdef CSPYDR_USE_LLVM

#include <string.h>

#include "ast/ast.h"
#include "io/log.h"
#include "llvm_codegen.h"
#include "llvm_includes.h"
#include "globals.h"

typedef struct 
{
    ASTProg_T* ast;
    
    LLVMModuleRef module;
    LLVMBuilderRef builder;

    LLVMValueRef current_fn_value;
    ASTObj_T* current_fn;
    LLVMBasicBlockRef current_block;
} LLVMCodegenData_T;

static void optimize_llvm(LLVMCodegenData_T* cg);
static char* llvm_emit_bc(LLVMCodegenData_T* cg, char* output_file);
static void llvm_link_bc(LLVMCodegenData_T* cg, char* bc_file, char* output_file);

static void init_llvm_codegen(LLVMCodegenData_T* cg, ASTProg_T* ast)
{
    memset(cg, 0, sizeof(LLVMCodegenData_T));
    cg->ast = ast;
    cg->module = LLVMModuleCreateWithName(ast->main_file_path);
    LLVMSetDataLayout(cg->module, "");
    cg->builder = LLVMCreateBuilder();
}

static void free_llvm_codegen(LLVMCodegenData_T* cg)
{
}

void generate_llvm(ASTProg_T *ast, char *output_file, bool print_code, bool is_silent)
{ 
    LLVMCodegenData_T cg;
    init_llvm_codegen(&cg, ast);

    char* target = LLVMGetDefaultTargetTriple();
    LLVMSetTarget(cg.module, target);
    if(!is_silent)
        LOG_OK_F(COLOR_BOLD_BLUE "  Generating" COLOR_RESET " LLVM-IR for " COLOR_BOLD_WHITE "%s" COLOR_RESET "\n", target);
    LLVMDisposeMessage(target);

    char* error = NULL;
    if(LLVMVerifyModule(cg.module, LLVMAbortProcessAction, &error))
    {
        LOG_ERROR_F(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " error while generating llvm-ir: %s\n", error);
        exit(1);
    }
    LLVMDisposeMessage(error);

    if(global.optimize)
        optimize_llvm(&cg);

    if(print_code)
    {
        char* code = LLVMPrintModuleToString(cg.module);
        LOG_INFO_F(COLOR_RESET "%s\n", code);
        LLVMDisposeMessage(code);
    }

    char* file = NULL;

    if(!global.do_assemble)
        goto finish;
    
    file = llvm_emit_bc(&cg, output_file);

    if(!global.do_link)
        goto finish;
    
    if(!is_silent)
        LOG_OK_F(COLOR_BOLD_BLUE "  Linking" COLOR_RESET "    %s\n", output_file);
    llvm_link_bc(&cg, file, output_file);

finish:
    if(file)
        free(file);
    free_llvm_codegen(&cg);
}

static void optimize_llvm(LLVMCodegenData_T* cg)
{
    LLVMPassManagerRef pass = LLVMCreatePassManager();
    LLVMAddInstructionCombiningPass(pass);
    LLVMAddMemCpyOptPass(pass);
    LLVMAddGVNPass(pass);
    LLVMAddCFGSimplificationPass(pass);
    LLVMAddVerifierPass(pass);

    LLVMRunPassManager(pass, cg->module);
    LLVMDisposePassManager(pass);
}

static char* llvm_emit_bc(LLVMCodegenData_T* cg, char* output_file)
{
    char* file = calloc(strlen(output_file) + 4, sizeof(char));
    sprintf(file, "%s.bc", output_file);

    if(LLVMWriteBitcodeToFile(cg->module, file))
    {
        LOG_ERROR_F(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " while trying to output llvm-ir to \"%s\"\n" COLOR_RESET, file);
        exit(1);
    }

    return file;
}

static void llvm_link_bc(LLVMCodegenData_T* cg, char* bc_file, char* output_file)
{
    
}

#endif