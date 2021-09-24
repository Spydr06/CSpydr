#include "llvm_codegen.h"
#include "../../io/io.h"
#include "../../io/log.h"
#include "../../ast/types.h"
#include "../../error/error.h"
#include "llvm_casts.h"

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/ErrorHandling.h>
#include <llvm-c/Types.h>
#include <llvm-c/Transforms/Scalar.h>
#include <llvm-c/Target.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Linker.h>

#include <string.h>

LLVMCodegenData_T* init_llvm_cg(ASTProg_T* ast)
{
    LLVMCodegenData_T* cg = malloc(sizeof(struct LLVM_CODEGEN_DATA_STRUCT));
    cg->ast = ast;
    cg->silent = false;
    cg->current_fn = NULL;
    cg->current_block = NULL;
    cg->current_fn_ast = NULL;
    cg->vars = init_list(sizeof(LLVMValueRef));
    cg->main_fn = NULL;
    cg->fns = init_list(sizeof(LLVMValueRef));

    LLVMStartMultithreaded();

    return cg;
}

void free_llvm_cg(LLVMCodegenData_T* cg)
{
    LLVMDisposeModule(cg->llvm_module);
    LLVMDisposeBuilder(cg->llvm_builder);

    free_list(cg->vars);
    free_list(cg->fns);
    free(cg);
}

void llvm_gen_code(LLVMCodegenData_T* cg)
{
    if(!cg->silent)
        LOG_OK(COLOR_BOLD_BLUE "  Generating" COLOR_RESET " LLVM-IR\n");

    cg->llvm_module = LLVMModuleCreateWithName(cg->ast->main_file_path);
    LLVMSetDataLayout(cg->llvm_module, "");

    char* defaultTarget = LLVMGetDefaultTargetTriple();
    LLVMSetTarget(cg->llvm_module, defaultTarget);
    LLVMDisposeMessage(defaultTarget);

    cg->llvm_builder = LLVMCreateBuilder();

    for(int i = 0; i < cg->ast->objs->size; i++)
        llvm_gen_obj(cg, cg->ast->objs->items[i]);

    for(int i = 0; i < cg->ast->objs->size; i++)
    {
        ASTObj_T* obj = cg->ast->objs->items[i];
        if(obj->kind == OBJ_FUNCTION && !obj->is_extern)
        {
            cg->current_fn_ast = obj;
            cg->current_fn = find_fn(cg, llvm_gen_identifier(cg, obj->id));
            llvm_gen_fn_body(cg, obj);
        }
    }

    char* error = NULL;
    bool is_invalid = LLVMVerifyModule(cg->llvm_module, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error);

    if(is_invalid)
        LOG_ERROR("Failed generating llvm-bytecode"); 
}

void llvm_print_code(LLVMCodegenData_T* cg)
{
    if(!cg->silent)
        LOG_OK(COLOR_BOLD_GREEN "->" COLOR_RESET " Generated LLVM Code:\n");
    LOG_INFO_F("%s\n", LLVMPrintModuleToString(cg->llvm_module));
}

static void llvm_optimize_module(LLVMCodegenData_T* cg)
{
    LLVMPassManagerRef pass = LLVMCreatePassManager();
    LLVMAddInstructionCombiningPass(pass);
    LLVMAddMemCpyOptPass(pass);
    LLVMAddGVNPass(pass);
    LLVMAddCFGSimplificationPass(pass);
    LLVMAddVerifierPass(pass);

    LLVMRunPassManager(pass, cg->llvm_module);
    LLVMDisposePassManager(pass);
}

void llvm_emit_code(LLVMCodegenData_T* cg, const char* target)
{
    llvm_optimize_module(cg);

    const char* bc_tmp = "%s.bc";
    char* target_bc = calloc(strlen(bc_tmp) + strlen(target) + 1, sizeof(char));
    sprintf(target_bc, bc_tmp, target);

    if(LLVMWriteBitcodeToFile(cg->llvm_module, target_bc) != 0)
    {
        LOG_ERROR_F("Error while trying to output LLVM-Bitcode to \"%s\":\n" COLOR_RESET, target_bc);
        exit(1);
    }

    free(target_bc);
}

void llvm_run_code(LLVMCodegenData_T *cg)
{
    llvm_optimize_module(cg);

    if(!cg->silent)
        LOG_OK(COLOR_BOLD_BLUE "  Executing " COLOR_RESET " LLVM-IR using MC-JIT\n");

    LLVMExecutionEngineRef llvm_engine;

    LLVMLinkInMCJIT();
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    
    char* error = NULL;
    if(LLVMCreateExecutionEngineForModule(&llvm_engine, cg->llvm_module, &error) != 0)
    {
        LOG_ERROR_F("Error while creating LLVM JIT Compiler:\n" COLOR_RESET "  -> %s\n", error);
        LLVMDisposeMessage(error);
        exit(1);
    }

    int exit_code = -1;
    if(cg->main_fn->args->size == 0)
    {
        int (*main_fn)(void) = (int(*)(void)) LLVMGetFunctionAddress(llvm_engine, "main");
        exit_code = main_fn();
    }   
    else if(cg->main_fn->args->size == 2)
    {
        int argc = 1;
        char** argv = calloc(1, sizeof(char*));
        argv[0] = "JIT";    // this would normally be the path to the executable

        int (*main_fn)(int, char**) = (int(*)(int, char**)) LLVMGetFunctionAddress(llvm_engine, "main");
        exit_code = main_fn(argc, argv);

        free(argv);
    }
    if(!cg->silent)
        LOG_INFO_F("[\"%s\" terminated with exit code %d]\n", cg->ast->main_file_path, exit_code);
}

char* llvm_gen_identifier(LLVMCodegenData_T* cg, ASTIdentifier_T* id)
{
    // temporary
    return id->callee;
}

LLVMValueRef find_id(LLVMCodegenData_T* cg, char* callee)
{
    for(size_t i = 0; i < cg->vars->size; i++)
        if(strcmp(LLVMGetValueName((LLVMValueRef)cg->vars->items[i]), callee) == 0)
            return cg->vars->items[i];
    return NULL;
}

LLVMValueRef find_fn(LLVMCodegenData_T* cg, char* callee)
{
    for(size_t i = 0; i < cg->fns->size; i++)
        if(strcmp(LLVMGetValueName((LLVMValueRef)cg->fns->items[i]), callee) == 0)
            return cg->fns->items[i];
    return NULL;
}

void llvm_exit_hook(void)
{
    LLVMShutdown();
}
