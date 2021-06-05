#include "llvm_codegen.h"
#include "../../io/io.h"
#include "../../io/log.h"
#include "../../ast/types.h"

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Types.h>

#include <string.h>

LLVMCodegenData_T* init_llvm_cg(ASTProg_T* ast, const char* target_bin)
{
    LLVMCodegenData_T* cg = malloc(sizeof(struct LLVM_CODEGEN_DATA_STRUCT));
    cg->ast = ast;
    cg->target_bin = target_bin;
    cg->print_ll = false;
    cg->silent = false;
    cg->current_fn = NULL;
    cg->current_block = NULL;
    cg->current_fn_ast = NULL;
    cg->vars = init_list(sizeof(LLVMValueRef));

    return cg;
}

void free_llvm_cg(LLVMCodegenData_T* cg)
{
    free_list(cg->vars);
    free(cg);
}

static LLVMValueRef llvm_gen_obj(LLVMCodegenData_T* cg, ASTObj_T* obj);

void llvm_gen_code(LLVMCodegenData_T* cg)
{
    if(!cg->silent)
        LOG_OK(COLOR_BOLD_BLUE "  Generating" COLOR_RESET " llvm-IR\n");

    cg->llvm_module = LLVMModuleCreateWithName(cg->ast->main_file_path);
    LLVMSetDataLayout(cg->llvm_module, "");
    LLVMSetTarget(cg->llvm_module, LLVMGetDefaultTargetTriple());

    cg->llvm_builder = LLVMCreateBuilder();

    for(int i = 0; i < cg->ast->objs->size; i++)
        llvm_gen_obj(cg, cg->ast->objs->items[i]);

    char* error = NULL;
    bool is_invalid = LLVMVerifyModule(cg->llvm_module, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error);

    if(is_invalid)
    {
        LOG_ERROR("Failed generating llvm-bytecode");
        return;   
    }

    char* out_data = LLVMPrintModuleToString(cg->llvm_module);

    if(cg->print_ll)
        LOG_INFO_F("%s\n", out_data);

    const char* ll_tmp = "%s.ll";
    char* target_ll = calloc(strlen(ll_tmp) + strlen(cg->target_bin) + 1, sizeof(char));
    sprintf(target_ll, ll_tmp, cg->target_bin);

    write_file(target_ll, out_data);
    LLVMDisposeMessage(out_data);

    free(target_ll);
    return;
}

static LLVMTypeRef llvm_gen_type(LLVMCodegenData_T* cg, ASTType_T* ty)
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
        case TY_F80:
            // TODO:
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
static LLVMValueRef llvm_gen_expr(LLVMCodegenData_T* cg, ASTNode_T* stmt);

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
    llvm_gen_stmt(cg, obj->body);

    LLVMVerifyFunction(cg->current_fn, LLVMPrintMessageAction);

    return fn;
}

static void llvm_gen_local(LLVMCodegenData_T* cg, ASTObj_T* local)
{
   //TODO:
}

static void llvm_gen_block(LLVMCodegenData_T* cg, ASTNode_T* node, char* name)
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
    for(size_t i = 0; i < node->locals->size; i++)
        llvm_gen_local(cg, node->locals->items[i]);

    if(cg->current_fn_ast->return_type == primitives[TY_VOID])
        LLVMBuildRetVoid(cg->llvm_builder);

    for(size_t i = 0; i < node->stmts->size; i++)
        llvm_gen_stmt(cg, node->stmts->items[i]);

    for(size_t i = 0; i < argc + node->locals->size; i++)
        cg->vars->size--;

    LLVMPositionBuilderAtEnd(cg->llvm_builder, prev_block);
    cg->current_block = prev_block;
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

static void llvm_gen_stmt(LLVMCodegenData_T* cg, ASTNode_T* node)
{
    switch(node->kind)
    {
        case ND_BLOCK:
            llvm_gen_block(cg, node, "entry");
            break;
        case ND_RETURN:
            llvm_gen_return(cg, node);
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

static LLVMValueRef llvm_gen_expr(LLVMCodegenData_T* cg, ASTNode_T* node)
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
        default:
            return NULL;
    }
}