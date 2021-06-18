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
    cg->print_ll = false;
    cg->silent = false;
    cg->current_fn = NULL;
    cg->current_block = NULL;
    cg->current_fn_ast = NULL;
    cg->vars = init_list(sizeof(LLVMValueRef));
    cg->main_fn = NULL;
    cg->fns = init_list(sizeof(LLVMValueRef));

    LLVMEnablePrettyStackTrace();

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

static LLVMValueRef find_fn(LLVMCodegenData_T* cg, char* callee);

static LLVMValueRef llvm_gen_obj(LLVMCodegenData_T* cg, ASTObj_T* obj);
static void llvm_gen_fn_body(LLVMCodegenData_T* cg, ASTObj_T* fn);

void llvm_gen_code(LLVMCodegenData_T* cg)
{
    if(!cg->silent)
        LOG_OK(COLOR_BOLD_BLUE "  Generating" COLOR_RESET " LLVM-IR\n");

    cg->llvm_module = LLVMModuleCreateWithName(cg->ast->main_file_path);
    LLVMSetDataLayout(cg->llvm_module, "");
    LLVMSetTarget(cg->llvm_module, LLVMGetDefaultTargetTriple());

    cg->llvm_builder = LLVMCreateBuilder();

    for(int i = 0; i < cg->ast->objs->size; i++)
        llvm_gen_obj(cg, cg->ast->objs->items[i]);

    for(int i = 0; i < cg->ast->objs->size; i++)
    {
        ASTObj_T* obj = cg->ast->objs->items[i];
        if(obj->kind == OBJ_FUNCTION)
        {
            cg->current_fn_ast = obj;
            cg->current_fn = find_fn(cg, obj->callee);
            llvm_gen_fn_body(cg, obj);
        }
    }

    char* error = NULL;
    bool is_invalid = LLVMVerifyModule(cg->llvm_module, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error);

    if(is_invalid)
        LOG_ERROR("Failed generating llvm-bytecode"); 
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
}

void llvm_emit_code(LLVMCodegenData_T* cg, const char* target)
{
    llvm_optimize_module(cg);

    if(cg->print_ll)
        LOG_INFO_F("%s\n", LLVMPrintModuleToString(cg->llvm_module));

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
        LOG_INFO_F("\"%s\" terminated with exit code %d.\n", cg->ast->main_file_path, exit_code);
}

LLVMTypeRef llvm_gen_type(LLVMCodegenData_T* cg, ASTType_T* ty)
{
    switch(ty->kind)
    {
        case TY_I8: case TY_I16: case TY_I32: case TY_I64:
        case TY_U8: case TY_U16: case TY_U32: case TY_U64:
            return LLVMIntType(ty->size << 3);
        case TY_F32:
            return LLVMFloatType();
        case TY_F64:
            return LLVMDoubleType();
            return NULL;
        case TY_BOOL:
            return LLVMInt1Type();
        case TY_CHAR:
            return LLVMInt8Type();
        case TY_VOID:
            return LLVMVoidType();
        case TY_PTR:
            return LLVMPointerType(llvm_gen_type(cg, ty->base), ty->base->size);
        case TY_ARR:
            return LLVMArrayType(llvm_gen_type(cg, ty->base), 0);
        case TY_ENUM:
            //TODO:
            return NULL;
        case TY_STRUCT:
            return NULL;
        case TY_UNDEF:
            //TODO:
            return NULL;
    }

    // satisfy -Wall
    return NULL;
}

static LLVMValueRef llvm_gen_global(LLVMCodegenData_T* cg, ASTObj_T* global);
static LLVMValueRef llvm_gen_fn(LLVMCodegenData_T* cg, ASTObj_T* obj);

static LLVMValueRef llvm_gen_obj(LLVMCodegenData_T* cg, ASTObj_T* obj)
{
    switch(obj->kind)
    {
        case OBJ_GLOBAL:
            return llvm_gen_global(cg, obj);
            break;
        case OBJ_LOCAL:
            break;
        case OBJ_FUNCTION:
            return llvm_gen_fn(cg, obj);
            break;
        case OBJ_FN_ARG:
            break;
        case OBJ_TYPEDEF:
            break;
    }
    return NULL;
}

static LLVMValueRef llvm_gen_global(LLVMCodegenData_T* cg, ASTObj_T* ast)
{
    LLVMTypeRef type = llvm_gen_type(cg, ast->data_type);
    LLVMValueRef global = LLVMAddGlobal(cg->llvm_module, type, ast->callee);

    list_push(cg->vars, global);

    return global;
}

static void llvm_gen_stmt(LLVMCodegenData_T* cg, ASTNode_T* stmt);
LLVMValueRef llvm_gen_expr(LLVMCodegenData_T* cg, ASTNode_T* stmt);
static LLVMBasicBlockRef llvm_begin_label(LLVMCodegenData_T* cg, char* name);
static void llvm_end_label(LLVMCodegenData_T* cg, LLVMBasicBlockRef prev_block);

static LLVMValueRef llvm_gen_fn(LLVMCodegenData_T* cg, ASTObj_T* obj)
{
    LLVMTypeRef return_type = llvm_gen_type(cg, obj->return_type);
    size_t argc = obj->args->size;

    LLVMTypeRef* arg_types = calloc(argc, sizeof(LLVMTypeRef));
    for(size_t i = 0; i < argc; i++)
    {
        arg_types[i] = llvm_gen_type(cg, ((ASTObj_T*) obj->args->items[i])->data_type);
    }   

    LLVMTypeRef fn_type = LLVMFunctionType(return_type, arg_types, argc, false);
    LLVMValueRef fn = LLVMAddFunction(cg->llvm_module, obj->callee, fn_type);

    cg->current_fn = fn;
    cg->current_fn_ast = obj;
    list_push(cg->fns, fn);

    if(strcmp(obj->callee, "main") == 0)
        cg->main_fn = obj;

    return fn;
}

static void llvm_gen_fn_body(LLVMCodegenData_T* cg, ASTObj_T* fn)
{
    LLVMBasicBlockRef prev_block = llvm_begin_label(cg, "entry");
    llvm_gen_stmt(cg, fn->body);
    llvm_end_label(cg, prev_block);

    LLVMVerifyFunction(cg->current_fn, LLVMPrintMessageAction);
}

static void llvm_gen_local(LLVMCodegenData_T* cg, ASTObj_T* local)
{
   //TODO:
}

static LLVMBasicBlockRef llvm_begin_label(LLVMCodegenData_T* cg, char* name)
{
    LLVMBasicBlockRef prev_block = cg->current_block;

    LLVMBasicBlockRef block = LLVMAppendBasicBlock(cg->current_fn, name);
    LLVMPositionBuilderAtEnd(cg->llvm_builder, block);

    size_t argc = cg->current_fn_ast->args->size;
    if(argc > 0)
    {
        LLVMValueRef* arg_allocs = calloc(argc, sizeof(LLVMValueRef));
        LLVMValueRef* arg_values = calloc(argc, sizeof(LLVMValueRef));
        LLVMGetParams(cg->current_fn, arg_values);

        for(size_t i = 0; i < argc; i++)
        {
            ASTObj_T* arg = cg->current_fn_ast->args->items[i];
            LLVMSetValueName(arg_values[i], arg->callee);
            arg_allocs[i] = LLVMBuildAlloca(cg->llvm_builder, llvm_gen_type(cg, arg->data_type), "");
            LLVMBuildStore(cg->llvm_builder, arg_values[i], arg_allocs[i]);
            list_push(cg->vars, arg_values[i]);
        }
    }

    return prev_block;
}

static void llvm_end_label(LLVMCodegenData_T* cg, LLVMBasicBlockRef prev_block)
{
    size_t argc = cg->current_fn_ast->args->size;
    for(size_t i = 0; i < argc; i++)
        cg->vars->size--;

    LLVMPositionBuilderAtEnd(cg->llvm_builder, prev_block);
    cg->current_block = prev_block;
}

static void llvm_gen_block(LLVMCodegenData_T* cg, ASTNode_T* node)
{   
    for(size_t i = 0; i < node->locals->size; i++)
        llvm_gen_local(cg, node->locals->items[i]);

    for(size_t i = 0; i < node->stmts->size; i++)
        llvm_gen_stmt(cg, node->stmts->items[i]);

    if(cg->current_fn_ast->return_type == primitives[TY_VOID])
        LLVMBuildRetVoid(cg->llvm_builder);
    
    for(size_t i = 0; i < node->locals->size; i++)
        cg->vars->size--;
}

static void llvm_gen_return(LLVMCodegenData_T* cg, ASTNode_T* node)
{
    if(node->return_val)
    {
        LLVMValueRef val = llvm_gen_expr(cg, node->return_val);
        LLVMBuildRet(cg->llvm_builder, val);
    } else
        LLVMBuildRetVoid(cg->llvm_builder);
}

static void llvm_gen_if(LLVMCodegenData_T* cg, ASTNode_T* node)
{
    LLVMBasicBlockRef true_block = LLVMAppendBasicBlock(cg->current_fn, "if.then");
    LLVMBasicBlockRef false_block = node->else_branch ? LLVMAppendBasicBlock(cg->current_fn, "if.else") : LLVMAppendBasicBlock(cg->current_fn, "if.else");

    LLVMValueRef cond = llvm_gen_expr(cg, node->condition);
    LLVMBuildCondBr(cg->llvm_builder, cond, true_block, false_block); 

    LLVMPositionBuilderAtEnd(cg->llvm_builder, true_block);
    llvm_gen_stmt(cg, node->if_branch);

    if(node->else_branch)
    {
        LLVMBuildBr(cg->llvm_builder, true_block);
        LLVMPositionBuilderAtEnd(cg->llvm_builder, false_block);
        llvm_gen_stmt(cg, node->else_branch);
    }
}

static void llvm_gen_loop(LLVMCodegenData_T* cg, ASTNode_T* node)
{

}

static void llvm_gen_stmt(LLVMCodegenData_T* cg, ASTNode_T* node)
{
    switch(node->kind)
    {
        case ND_BLOCK:
            llvm_gen_block(cg, node);
            break;
        case ND_RETURN:
            llvm_gen_return(cg, node);
            break;
        case ND_IF:
            llvm_gen_if(cg, node);
            break;
        case ND_LOOP:
            llvm_gen_loop(cg, node);
            break;
        default:
            break;
    }
}

static LLVMValueRef find_id(LLVMCodegenData_T* cg, char* callee)
{
    for(size_t i = 0; i < cg->vars->size; i++)
        if(strcmp(LLVMGetValueName((LLVMValueRef)cg->vars->items[i]), callee) == 0)
            return cg->vars->items[i];
    return NULL;
}

static LLVMValueRef find_fn(LLVMCodegenData_T* cg, char* callee)
{
    for(size_t i = 0; i < cg->fns->size; i++)
        if(strcmp(LLVMGetValueName((LLVMValueRef)cg->fns->items[i]), callee) == 0)
            return cg->fns->items[i];
    return NULL;
}

static LLVMValueRef llvm_gen_call(LLVMCodegenData_T* cg, ASTNode_T* call)
{
    LLVMValueRef fn = find_fn(cg, call->expr->callee);
    size_t argc = call->args->size;
    LLVMValueRef* args = calloc(argc, sizeof(LLVMValueRef));

    for(int i = 0; i < argc; i++)
        args[i] = llvm_gen_expr(cg, call->args->items[i]);
    
    return LLVMBuildCall(cg->llvm_builder, fn, args, (unsigned) argc, "");
}

static LLVMValueRef llvm_gen_op(LLVMCodegenData_T* cg, ASTNode_T* op)
{
    LLVMValueRef left =  llvm_gen_expr(cg, op->left);
    LLVMValueRef right = llvm_gen_expr(cg, op->right);

    switch(op->kind)
    {
        case ND_ADD:
            return LLVMBuildBinOp(cg->llvm_builder, LLVMAdd, left, right, "add");
        case ND_SUB:
            return LLVMBuildBinOp(cg->llvm_builder, LLVMSub, left, right, "sub");
        case ND_MUL:
            return LLVMBuildBinOp(cg->llvm_builder, LLVMMul, left, right, "mul");
        case ND_DIV:
            return LLVMBuildBinOp(cg->llvm_builder, LLVMFDiv, left, right, "fdiv");
        default:
            throw_error(ERR_UNDEFINED, op->tok, "undefined op type %d", op->kind);
    }

    return NULL;
}

static LLVMValueRef llvm_gen_cmp(LLVMCodegenData_T* cg, ASTNode_T* op)
{
    //TODO:

    return NULL;
}

static LLVMValueRef llvm_gen_cast(LLVMCodegenData_T* cg, ASTNode_T* cast)
{
    LLVMCastFn cast_fn = llvm_cast_map[cast->left->data_type->kind][cast->data_type->kind];
    if(!cast_fn)
        throw_error(ERR_ILLEGAL_TYPE_CAST, cast->tok, "no cast function for casting from `%s` to `%s` found", cast->left->data_type->tok->value, cast->data_type->tok->value);
        
    return cast_fn(cg, cast);
}

LLVMValueRef llvm_gen_expr(LLVMCodegenData_T* cg, ASTNode_T* node)
{
    switch(node->kind) {
        case ND_INT:
            return LLVMConstInt(LLVMInt32Type(), node->int_val, false);
        case ND_FLOAT:
            return LLVMConstReal(LLVMFloatType(), node->float_val);
        case ND_CHAR:
            return LLVMConstInt(LLVMInt8Type(), (int8_t) node->char_val, false);
        case ND_BOOL:
            return LLVMConstInt(LLVMInt1Type(), node->bool_val, false);
        case ND_ID:
            return find_id(cg, node->callee);
        case ND_CALL:
            return llvm_gen_call(cg, node);
        case ND_ADD:
        case ND_SUB:
        case ND_MUL:
        case ND_DIV:
            return llvm_gen_op(cg, node);
        case ND_NEG:
            return LLVMBuildNeg(cg->llvm_builder, llvm_gen_expr(cg, node->left), "neg");
        case ND_NOT:
            return LLVMBuildNot(cg->llvm_builder, llvm_gen_expr(cg, node->left), "not");
        case ND_BIT_NEG:
            //TODO:
            return NULL;
        case ND_EQ:
        case ND_NE:
        case ND_GT:
        case ND_GE:
        case ND_LT:
        case ND_LE:
            return llvm_gen_cmp(cg, node);
        case ND_CAST:
            return llvm_gen_cast(cg, node);
        default:
            return NULL;
    }
}