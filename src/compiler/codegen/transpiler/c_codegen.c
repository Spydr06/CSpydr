#include "c_codegen.h"

#include "../../io/log.h"
#include "../../io/io.h"
#include "../../error/error.h"

#include <llvm-c/Target.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "../../platform/platform_bindings.h"

#define CC "gcc"
#define CC_FLAGS "-O3 -Wall -fPIC"

#define DEFAULT_IMPORTS "#include <stdbool.h>\n" \
                        "#include <stdlib.h>"

static const char* primitive_to_c_type[TY_UNDEF + 1] = {
    [TY_I8]  = "int8_t" ,
    [TY_I16] = "int16_t",
    [TY_I32] = "int32_t",
    [TY_I64] = "int64_t",

    [TY_U8]  = "u_int8_t" ,
    [TY_U16] = "u_int16_t",
    [TY_U32] = "u_int32_t",
    [TY_U64] = "u_int64_t",

    [TY_F32] = "float",
    [TY_F64] = "double",

    [TY_BOOL] = "bool",
    [TY_VOID] = "void",
    [TY_CHAR] = "char",
};

CCodegenData_T* init_c_cg(ASTProg_T* ast)
{
    CCodegenData_T* cg = malloc(sizeof(struct C_CODEGEN_DATA_STRUCT));
    cg->ast = ast;
    cg->print_c = false;
    cg->silent = false;

    cg->code_buffer = open_memstream(&cg->buf, &cg->buf_len);

    return cg;
}

void free_c_cg(CCodegenData_T* cg)
{
    free(cg);
}

static void println(CCodegenData_T* cg, char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vfprintf(cg->code_buffer, fmt, va);
    va_end(va);
    fprintf(cg->code_buffer, "\n");
}

static void print(CCodegenData_T* cg, char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vfprintf(cg->code_buffer, fmt, va);
    va_end(va);
}

static void write_code(CCodegenData_T* cg, const char* target_bin);
static void run_compiler(CCodegenData_T* cg,  const char* target_bin);

static void c_gen_obj_decl(CCodegenData_T* cg, ASTObj_T* obj);
static void c_gen_obj(CCodegenData_T* cg, ASTObj_T* obj);

void c_gen_code(CCodegenData_T* cg, const char* target)
{
    if(!cg->silent)
        LOG_OK(COLOR_BOLD_BLUE "  Generating" COLOR_RESET " C code using " COLOR_BOLD_WHITE CC COLOR_RESET "\n");

    println(cg, "#include <stdlib.h>");
    for(size_t i = 0; i < cg->ast->objs->size; i++)
        c_gen_obj_decl(cg, cg->ast->objs->items[i]);
    
    for(size_t i = 0; i < cg->ast->objs->size; i++)
        c_gen_obj(cg, cg->ast->objs->items[i]);

    write_code(cg, target);

    if(cg->print_c)
        LOG_INFO_F("%s", cg->buf);
    
    run_compiler(cg, target);
}

static void run_compiler(CCodegenData_T* cg, const char* target_bin)
{

    static char* compiler_cmd_tmp = CC " " CC_FLAGS " %s.c -o %s";
    char* compiler_cmd = calloc(strlen(compiler_cmd_tmp) + strlen(target_bin) * 2 + 1, sizeof(char));
    sprintf(compiler_cmd, compiler_cmd_tmp, target_bin, target_bin);

    char* feedback = sh(compiler_cmd);
    if(!cg->silent)
        LOG_INFO_F("%s", feedback);

    free(compiler_cmd);
}

static void write_code(CCodegenData_T* cg, const char* target_bin)
{
    const char* file_tmp = "%s.c";
    char* c_file_path = calloc(strlen(file_tmp) + strlen(target_bin) + 1, sizeof(char));
    sprintf(c_file_path, file_tmp, target_bin);

    fclose(cg->code_buffer);

    FILE* out = open_file(c_file_path);
    fwrite(cg->buf, cg->buf_len, 1, out);
    fclose(out);

    free(c_file_path);
}

void run_c_code(CCodegenData_T* cg, const char* bin)
{
    if(!cg->silent)
        LOG_OK_F(COLOR_BOLD_BLUE "  Executing " COLOR_RESET " %s\n", bin);
    
    const char* cmd_tmp = "." DIRECTORY_DELIMS "%s";
    char* cmd = calloc(strlen(cmd_tmp) + strlen(bin) + 1, sizeof(char));
    sprintf(cmd, cmd_tmp, bin);

    char* feedback = sh(cmd);
    if(!cg->silent)
        LOG_OK_F("%s", feedback);
    
#ifdef __linux
    if(!cg->silent)
    {
        int exit_code = atoi(sh("echo $?"));
        LOG_INFO_F("\"%s\" terminated with exit code %d.\n", cg->ast->main_file_path ,exit_code);
    }
#endif

    free(cmd);
}

static void c_gen_expr(CCodegenData_T* cg, ASTNode_T* node);
static void c_gen_stmt(CCodegenData_T* cg, ASTNode_T* node);

static void c_gen_type(CCodegenData_T* cg, ASTType_T* ty)
{
    if(primitive_to_c_type[ty->kind])
        print(cg, "%s", primitive_to_c_type[ty->kind]);
    else
        switch(ty->kind)
        {
            case TY_PTR:
                c_gen_type(cg, ty->base);
                print(cg, "*");
                break;
            case TY_ARR:
                c_gen_type(cg, ty->base);
                if(ty->num_indices)
                {
                    print(cg, "[");
                    c_gen_expr(cg, ty->num_indices);
                    print(cg, "]");
                }
                else
                    print(cg, "[]");
                break;
            default:
                throw_error(ERR_MISC, ty->tok, "Types with kind %d are currently not supported.", ty->kind);
                break;
        }
}

static void c_gen_fn_arg_list(CCodegenData_T* cg, List_T* args)
{
    for(size_t i = 0; i < args->size; i++)
    {
        ASTObj_T* arg = args->items[i];
        c_gen_type(cg, arg->data_type);
        print(cg, " %s%s", arg->callee, i < args->size - 1 ? "," : "");
    }
}

static void c_gen_obj_decl(CCodegenData_T* cg, ASTObj_T* obj)
{
    switch(obj->kind)
    {
        case OBJ_GLOBAL:
            c_gen_type(cg, obj->data_type);
            print(cg, " %s", obj->callee);
            if(obj->value)
            {
                print(cg, "=");
                c_gen_expr(cg, obj->value);
            }
            println(cg, ";");
            break;
        case OBJ_FUNCTION:
            c_gen_type(cg, obj->return_type);
            print(cg, " %s(", obj->callee);
            if(obj->args)
                c_gen_fn_arg_list(cg, obj->args);
            println(cg, ");");
            break;
        default:
            break;
    }
}

static void c_gen_obj(CCodegenData_T* cg, ASTObj_T* obj)
{
    switch(obj->kind)
    {
        case OBJ_FUNCTION:
            c_gen_type(cg, obj->return_type);
            print(cg, " %s(", obj->callee);
            if(obj->args)
                c_gen_fn_arg_list(cg, obj->args);
            println(cg, "){");
            c_gen_stmt(cg, obj->body);
            println(cg, "}");
            break;
        default:
            break;
    }
}

static void c_gen_expr(CCodegenData_T* cg, ASTNode_T* node)
{
    switch(node->kind)
    {
        case ND_INT:
            print(cg, "%d", node->int_val);
            break;
        case ND_FLOAT:
            print(cg, "%f", node->float_val);
            break;
        case ND_BOOL:
            print(cg, node->bool_val ? "true" : "false");
            break;
        case ND_CHAR:
            print(cg, "'%c'", node->char_val);
            break;
        case ND_STR:
            print(cg, "\"%s\"", node->str_val);
            break;
        case ND_ID: 
            print(cg, "%s", node->callee);
            break;
        case ND_CALL:
            print(cg, "%s(", node->callee);
            for(int i = 0; i < node->args->size; i++)
            {
                c_gen_expr(cg, node->args->items[i]);
                print(cg, i < node->args->size - 1 ? "," : ")");
            }
            break;
        case ND_ASSIGN:
        case ND_ADD:
        case ND_SUB:
        case ND_DIV:
        case ND_MUL:
        case ND_EQ:
        case ND_LT:
        case ND_LE:
        case ND_GT:
        case ND_GE:
        case ND_NE:
            c_gen_expr(cg, node->left);
            print(cg, node->tok->value);
            c_gen_expr(cg, node->right);
            break;
        case ND_NEG:
        case ND_BIT_NEG:
        case ND_NOT:
            print(cg, node->tok->value);
            c_gen_expr(cg, node->right);
            break;
        case ND_INC:
        case ND_DEC:
            c_gen_expr(cg, node->left);
            print(cg, node->tok->value);
            break;
        default:
            throw_error(ERR_MISC, node->tok, "Expressions of type %d are currently not supported", node->kind);
    }
}

static void c_gen_local(CCodegenData_T* cg, ASTObj_T* obj)
{
    c_gen_type(cg, obj->data_type);
    println(cg, " %s;", obj->callee);
}

static void c_gen_stmt(CCodegenData_T* cg, ASTNode_T* node)
{
    switch(node->kind)
    {
        case ND_RETURN:
            print(cg, "return ");
            if(node->return_val)
                c_gen_expr(cg, node->return_val);
            println(cg, ";");
            break;
        case ND_BLOCK:
            println(cg, "{");
            for(size_t i = 0; i < node->locals->size; i++)
                c_gen_local(cg, node->locals->items[i]);
            for(size_t i = 0; i < node->stmts->size; i++)
                c_gen_stmt(cg, node->stmts->items[i]);
            println(cg, "}");
            break;
        case ND_IF:
            print(cg, "if(");
            c_gen_expr(cg, node->condition);
            print(cg, ") ");
            c_gen_stmt(cg, node->if_branch);
            if(node->else_branch)
            {
                print(cg, "else ");
                c_gen_stmt(cg, node->else_branch);
            }
            break;
        case ND_LOOP:
            print(cg, "while(");
            c_gen_expr(cg, node->condition);
            print(cg, ") ");
            c_gen_stmt(cg, node->body);
            break;
        case ND_EXPR_STMT:
            c_gen_expr(cg, node->expr);
            println(cg, ";\n");
            break;
        case ND_NOOP:
            break;
        default:
            break;
    }
}