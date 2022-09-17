#ifdef CSPYDR_USE_LLVM

#include <string.h>
#include <stdarg.h>

#include "list.h"
#include "error/error.h"
#include "mem/mem.h"
#include "../codegen_utils.h"
#include "ast/ast.h"
#include "ast/ast_iterator.h"
#include "io/log.h"
#include "llvm_codegen.h"
#include "llvm_includes.h"
#include "globals.h"
#include "timer/timer.h"

#define GET_GENERATOR(args) LLVMCodegenData_T* cg = va_arg(args, LLVMCodegenData_T*)

typedef struct 
{
    ASTProg_T* ast;
    
    LLVMModuleRef module;
    LLVMBuilderRef builder;

    LLVMValueRef current_fn_value;
    ASTObj_T* current_fn;
    LLVMBasicBlockRef current_block;

    LLVMValueRef* current_value;
} LLVMCodegenData_T;

static void optimize_llvm(LLVMCodegenData_T* cg);
static char* llvm_emit_bc(LLVMCodegenData_T* cg, char* output_file);
static void llvm_link_bc(LLVMCodegenData_T* cg, char* bc_file, char* output_file);

static void gen_global(ASTObj_T* global, va_list args);
static void gen_enum(ASTObj_T* tydef, va_list args);

static ASTIteratorList_T iterator = {
    .obj_end_fns = {
        [OBJ_GLOBAL] = gen_global,
        [OBJ_TYPEDEF] = gen_enum
    }
};

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

i32 llvm_codegen_pass(ASTProg_T* ast)
{
    timer_start("llvm code generation");
    generate_llvm(ast, global.target, global.print_code, global.silent);
    timer_stop();

    return 0;
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

    ast_iterate(&iterator, ast, &cg);

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

static char* llvm_gen_identifier(LLVMCodegenData_T* cg, ASTIdentifier_T* id)
{
    char* str = gen_identifier(id, ".", false);
    mem_add_ptr(str);
    return str;
}

static LLVMTypeRef llvm_gen_type(LLVMCodegenData_T* cg, ASTType_T* type)
{
    type = unpack(type);
    switch(type->kind)
    {
        case TY_I8:
        case TY_U8:
        case TY_BOOL:
        case TY_CHAR:
            return LLVMInt8Type();
        case TY_I16:
        case TY_U16:
            return LLVMInt16Type();
        case TY_I32:
        case TY_U32:
        case TY_ENUM:
            return LLVMInt32Type();
        case TY_I64:
        case TY_U64:
            return LLVMInt64Type();
        case TY_F32:
            return LLVMFloatType();
        case TY_F64:
            return LLVMDoubleType();
        case TY_F80:
            return LLVMFP128Type();
        case TY_VOID:
            return LLVMVoidType();
        case TY_PTR:
        case TY_VLA:
            return LLVMPointerType(llvm_gen_type(cg, type->base), 8);
        case TY_C_ARRAY:
            return LLVMArrayType(llvm_gen_type(cg, type->base), type->num_indices);
        case TY_FN:
            {
                LLVMTypeRef* params = calloc(type->arg_types->size, sizeof(LLVMTypeRef));
                for(size_t i = 0; i < type->arg_types->size; i++)
                {
                    ASTType_T* ty = type->arg_types->items[i];
                    params[i] = llvm_gen_type(cg, ty);
                }
                mem_add_ptr(params);

                return LLVMPointerType(
                    LLVMFunctionType(
                        llvm_gen_type(cg, type->base), 
                        params, 
                        type->arg_types->size, 
                        type->is_variadic
                    ), 8
                );
            }
        case TY_STRUCT:
            {
                LLVMTypeRef* elements = calloc(type->members->size, sizeof(LLVMTypeRef));
                for(size_t i = 0; i < type->members->size; i++)
                {
                    ASTNode_T* member = type->members->items[i];
                    elements[i] = llvm_gen_type(cg, member->data_type);
                }
                mem_add_ptr(elements);

                return LLVMStructType(elements, type->members->size, false);    // todo: union support
            }
        
        case TY_ARRAY:
            {
                LLVMTypeRef* elements = calloc(2, sizeof(LLVMTypeRef));
                mem_add_ptr(elements);
                elements[0] = LLVMInt64Type();
                elements[1] = LLVMArrayType(llvm_gen_type(cg, type->base), type->num_indices);

                return LLVMStructType(elements, 2, false); // arrays are two parts: the size and the data
            }
        
        case TY_UNDEF:
        case TY_TYPEOF:
        case TY_TEMPLATE:
        case TY_KIND_LEN:
            unreachable();
            break;
    }
    return LLVMInt32Type();
}

static LLVMValueRef gen_global_initializer(LLVMCodegenData_T* cg, ASTNode_T* node, LLVMTypeRef type)
{
    switch(node->kind)
    {
        case ND_NIL:
            return LLVMConstNull(type);
        case ND_STR:
            return LLVMConstString(node->str_val, strlen(node->str_val), false);
        case ND_CHAR:
            return LLVMConstInt(type, node->int_val, false);
        case ND_BOOL:
            return LLVMConstInt(type, node->bool_val, false);
        case ND_INT:
            return LLVMConstInt(type, node->int_val, true);
        case ND_LONG:
            return LLVMConstInt(type, node->long_val, true);
        case ND_ULONG:
            return LLVMConstInt(type, node->ulong_val, false);
        case ND_FLOAT:
            return LLVMConstReal(type, node->float_val);
        case ND_DOUBLE:
            return LLVMConstReal(type, node->double_val);
        case ND_CLOSURE:
            return gen_global_initializer(cg, node->expr, type);
        case ND_CAST:
            return gen_global_initializer(cg, node->left, llvm_gen_type(cg, node->data_type));
        case ND_NEG:
            return LLVMConstNeg(gen_global_initializer(cg, node->left, type));
        
        case ND_ARRAY:
        case ND_STRUCT:
            throw_error(ERR_CODEGEN, node->tok, "not implemented");
            break;

        default:
            throw_error(ERR_CODEGEN, node->tok, "cannot generate relocation for `%s` (%d)", node->tok->value, node->kind);
    }

    return LLVMConstNull(type);
}

static void gen_global(ASTObj_T* global, va_list args)
{
    GET_GENERATOR(args);
    LLVMTypeRef type = llvm_gen_type(cg, global->data_type);
    LLVMValueRef value = LLVMAddGlobal(cg->module, type, llvm_gen_identifier(cg, global->id));
    if(!global->is_extern && should_emit(global))
        LLVMSetInitializer(value, gen_global_initializer(cg, global->value, type));
}

static void gen_enum(ASTObj_T* tydef, va_list args)
{
    GET_GENERATOR(args);

    if(!tydef->data_type || tydef->data_type->kind != TY_ENUM)
        return;
    LLVMTypeRef type = LLVMInt32Type();
    
    for(size_t i = 0; i < tydef->data_type->members->size; i++)
    {
        ASTObj_T* member = tydef->data_type->members->items[i];
        LLVMValueRef value = LLVMAddGlobal(cg->module, type, llvm_gen_identifier(cg, member->id));
        LLVMSetInitializer(value, gen_global_initializer(cg, member->value, type));
    }
}

void llvm_exit_hook(void) {
    LLVMShutdown();
}

#endif