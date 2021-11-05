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
#include "../../ast/mem/ast_mem.h"

char* cc = DEFAULT_CC;
char* cc_flags = DEFAULT_CC_FLAGS;

const char* default_header_code =
    "#include<stdarg.h>\n"
    "typedef _Bool bool;\n"
    "#define true ((bool) 1)\n"
    "#define false ((bool) 0)\n"
    "#define NULL ((void*) 0)\n"
    "#define len(n) (n?((unsigned long)(sizeof(n)/sizeof(n[0]))):0)\n"
;

#define len(n) (n ? ((unsigned long) (sizeof(n) / sizeof(n[0]))) : 0)

static const char* primitive_to_c_type[TY_UNDEF + 1] = {
    [TY_I8]  = "signed char" ,
    [TY_I16] = "signed short",
    [TY_I32] = "signed int",
    [TY_I64] = "signed long",

    [TY_U8]  = "unsigned char" ,
    [TY_U16] = "unsigned short",
    [TY_U32] = "unsigned int",
    [TY_U64] = "unsigned long",

    [TY_F32] = "float",
    [TY_F64] = "double",
    [TY_F80] = "long double",

    [TY_BOOL] = "bool",
    [TY_VOID] = "void",
    [TY_CHAR] = "char",
};

void init_c_cg(CCodegenData_T* cg, ASTProg_T* ast)
{
    cg->ast = ast;
    cg->print_c = false;
    cg->silent = false;

    cg->code_buffer = open_memstream(&cg->buf, &cg->buf_len);
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

static void c_gen_typedefs(CCodegenData_T* cg, ASTObj_T* obj);

static char* c_gen_identifier(CCodegenData_T* cg, ASTIdentifier_T* id);

static void write_code(CCodegenData_T* cg, const char* target_bin);
static void run_compiler(CCodegenData_T* cg,  const char* target_bin);

static void c_gen_obj_decl(CCodegenData_T* cg, ASTObj_T* obj);
static void c_gen_obj(CCodegenData_T* cg, ASTObj_T* obj);

static void c_gen_expr(CCodegenData_T* cg, ASTNode_T* node);
static void c_gen_stmt(CCodegenData_T* cg, ASTNode_T* node);

static void c_gen_type(CCodegenData_T* cg, ASTType_T* ty, char* struct_name);
static void c_gen_lambda_fn(CCodegenData_T* cg, ASTNode_T* lambda);
static void c_gen_array_brackets(CCodegenData_T* cg, ASTType_T* ty);
static void c_gen_tuple_struct(CCodegenData_T* cg, ASTType_T* tuple);

void c_gen_code(CCodegenData_T* cg, const char* target)
{
    if(!cg->silent)
        LOG_OK_F(COLOR_BOLD_BLUE "  Generating" COLOR_RESET " C code using " COLOR_BOLD_WHITE "%s" COLOR_RESET "\n", cc);

    /*
     * C code generation is split into many parts.
     * This is necessary, because, different to C,
     * the order of functions, typedefs, global
     * variables, etc. does not matter in CSpydr.
     * Therefore we have to "sort" all the different
     * sections of a program and emit them seperately.
     */

    println(cg, (char*) default_header_code);

    // generate typedefs
    for(size_t i = 0; i < cg->ast->objs->size; i++)
    {
        c_gen_typedefs(cg, cg->ast->objs->items[i]);
    }

    // declare all tuple structs
    for(size_t i = 0; i < cg->ast->tuple_structs->size; i++)
        c_gen_tuple_struct(cg, cg->ast->tuple_structs->items[i]);

    // declare all objects first
    for(size_t i = 0; i < cg->ast->objs->size; i++)
        c_gen_obj_decl(cg, cg->ast->objs->items[i]);

    // emit all lambda templates
    for(size_t i = 0; i < cg->ast->lambda_literals->size; i++)
        c_gen_lambda_fn(cg, cg->ast->lambda_literals->items[i]);
    
    // finally, emit every function containing the *actual* code
    for(size_t i = 0; i < cg->ast->objs->size; i++)
        c_gen_obj(cg, cg->ast->objs->items[i]);

    write_code(cg, target);

    if(cg->print_c)
        LOG_INFO_F("%s", cg->buf);
    
    run_compiler(cg, target);
}

static void run_compiler(CCodegenData_T* cg, const char* target_bin)
{
    char c_source_file[BUFSIZ] = {'\0'};
    sprintf(c_source_file, "%s" DIRECTORY_DELIMS CACHE_DIR DIRECTORY_DELIMS "%s.c", get_home_directory(), target_bin);

    char* args[] = // TODO: enable cc_flags again
    {
        cc,
        c_source_file,
        "-o",
        (char*) target_bin,
        "-lglfw",
        "-lGL",
        NULL
    };

    int exit_code = subprocess(args[0], args, false);

    if(exit_code != 0)
    {
        LOG_ERROR_F("error compiling code using %s. (exit code %d)\n", cc, exit_code);
        exit(1);
    }
}

static void write_code(CCodegenData_T* cg, const char* target_bin)
{
    char* homedir = get_home_directory();
    char cache_dir[BUFSIZ] = {'\0'};
    sprintf(cache_dir, "%s" DIRECTORY_DELIMS CACHE_DIR DIRECTORY_DELIMS, homedir);

    if(make_dir(cache_dir))
    {
        LOG_ERROR("error creating cache directory `" DIRECTORY_DELIMS CACHE_DIR DIRECTORY_DELIMS "`.\n");
        exit(1);
    }

    char c_file_path[BUFSIZ * 2] = {'\0'};
    sprintf(c_file_path, "%s" DIRECTORY_DELIMS "%s.c", cache_dir, target_bin);

    fclose(cg->code_buffer);

    FILE* out = open_file(c_file_path);
    fwrite(cg->buf, cg->buf_len, 1, out);
    fclose(out);
}

void run_c_code(CCodegenData_T* cg, const char* bin)
{
    if(!cg->silent)
        LOG_OK_F(COLOR_BOLD_BLUE "  Executing " COLOR_RESET " %s\n", bin);
    
    const char* cmd_tmp = "." DIRECTORY_DELIMS "%s";
    char cmd[BUFSIZ];
    memset(cmd, '\0', sizeof cmd);
    sprintf(cmd, cmd_tmp, bin);

    last_exit_code = subprocess(cmd, (char* const[]){cmd, NULL}, !cg->silent);
}

static void c_gen_type(CCodegenData_T* cg, ASTType_T* ty, char* struct_name)
{
    if(ty->is_constant)
        print(cg, "const ");

    if(ty->is_volatile)
        print(cg, "volatile ");

    if(ty->is_complex)
        print(cg, "_Complex ");

    if(ty->is_atomic)
        print(cg, "_Atomic ");

    if(primitive_to_c_type[ty->kind])
        print(cg, "%s", primitive_to_c_type[ty->kind]);
    else
        switch(ty->kind)
        {
            case TY_LAMBDA:
                c_gen_type(cg, ty->base, "");
                print(cg, " (*%s)(", struct_name);
                for(size_t i = 0; i < ty->arg_types->size; i++)
                {
                    c_gen_type(cg, ty->arg_types->items[i], "");
                    print(cg, i == ty->arg_types->size - 1 ? "" : ",");
                }
                print(cg, ")");
                break;
            case TY_PTR:
                c_gen_type(cg, ty->base, struct_name);
                print(cg, "*");
                break;
            case TY_ARR:
                c_gen_type(cg, ty->base, struct_name);
                /*if(ty->num_indices)
                {
                    print(cg, "[");
                    c_gen_expr(cg, ty->num_indices);
                    print(cg, "]");
                }
                else
                    print(cg, "[]");*/
                break;
            case TY_ENUM:
                print(cg, "enum {");

                for(size_t i = 0; i < ty->members->size; i++)
                {   
                    ASTObj_T* member = ty->members->items[i];
                    char* callee = c_gen_identifier(cg, member->id);
                    if(member->value)
                    {
                        print(cg, "%s=", callee);
                        c_gen_expr(cg, member->value);
                        print(cg, ",");
                    } else
                        print(cg, "%s,", callee);
                }

                print(cg, "}");
                break;
            case TY_STRUCT:
                println(cg, "struct %s {", struct_name);

                for(size_t i = 0; i < ty->members->size; i++)
                {
                    ASTNode_T* member = ty->members->items[i];
                    c_gen_type(cg, member->data_type, struct_name);
                    print(cg, " %s", c_gen_identifier(cg, member->id)); 
                    if(member->data_type->kind == TY_ARR)
                        c_gen_array_brackets(cg, member->data_type);
                    println(cg, ";");
                }

                print(cg, "}");
                break;
            case TY_TUPLE:
                print(cg, "struct %s", c_gen_identifier(cg, ty->id));
                break;
            case TY_UNDEF:
                if(struct_name && strcmp(c_gen_identifier(cg, ty->id), struct_name) == 0)
                    print(cg, "struct ");
                print(cg, c_gen_identifier(cg, ty->id));
                break;
            case TY_OPAQUE_STRUCT:
                print(cg, "struct %s", c_gen_identifier(cg, ty->id));
                break;
            default:
                throw_error(ERR_MISC, ty->tok, "Types with kind %d are currently not supported.", ty->kind);
                break;
        }
}

static void c_gen_tuple_struct(CCodegenData_T* cg, ASTType_T* tuple)
{
    print(cg, "struct %s{", c_gen_identifier(cg, tuple->id));
    for(size_t i = 0; i < tuple->arg_types->size; i++)
    {
        c_gen_type(cg, tuple->arg_types->items[i], "");
        print(cg, " _%ld; ", i);
    }
    println(cg, "};");
}

static void c_gen_array_brackets(CCodegenData_T* cg, ASTType_T* ty)
{
    print(cg, "[");
    if(ty->num_indices)
        c_gen_expr(cg, ty->num_indices);
    print(cg, "]");

    if(ty->base && ty->base->kind == TY_ARR)
        c_gen_array_brackets(cg, ty->base);
}

static void c_gen_typedefs(CCodegenData_T* cg, ASTObj_T* obj)
{
    switch(obj->kind)
    {
        case OBJ_TYPEDEF:
            print(cg, "typedef ");
            char* callee = c_gen_identifier(cg, obj->id);
            if(obj->data_type->kind == TY_STRUCT)
                print(cg, "struct %s", callee);
            else
                c_gen_type(cg, obj->data_type, callee);
        
            if(obj->data_type->kind == TY_LAMBDA)
                println(cg, ";");
            else
                println(cg, " %s;", callee);
            break;
        case OBJ_NAMESPACE:
            for(size_t i = 0; i < obj->objs->size; i++)
                c_gen_typedefs(cg, obj->objs->items[i]);
            break;
        default:   
            break;
    }
}

static bool c_gen_fn_arg_list(CCodegenData_T* cg, List_T* args)
{
    if(args->size == 0) 
    {
        print(cg, "void");
        return false;
    }
    for(size_t i = 0; i < args->size; i++)
    {
        ASTObj_T* arg = args->items[i];
     
        char* callee = c_gen_identifier(cg, arg->id);

        if(arg->data_type->kind == TY_VA_LIST)
        {
            print(cg, "...");
            return true;    // va lists are always the last argument
        }
        else
        {
            if(arg->data_type->kind == TY_LAMBDA)
                c_gen_type(cg, arg->data_type, callee);
            else
            {   
                c_gen_type(cg, arg->data_type, "");
                print(cg, " %s", callee);
            }

            if(arg->data_type->kind == TY_ARR)
                c_gen_array_brackets(cg, arg->data_type);

            print(cg, "%s", i < args->size - 1 ? "," : "");
        }
    }
    return false;
}

static void c_gen_obj_decl(CCodegenData_T* cg, ASTObj_T* obj)
{
    if(obj->is_extern)
        print(cg, "extern ");

    char* obj_callee = c_gen_identifier(cg, obj->id);

    switch(obj->kind)
    {
        case OBJ_GLOBAL:
            if(obj->is_constant)
                print(cg, "const ");

            c_gen_type(cg, obj->data_type, "");
            print(cg, " %s", obj_callee);
            if(obj->data_type->kind == TY_ARR)
                c_gen_array_brackets(cg, obj->data_type);
            if(obj->value)
            {
                print(cg, "=");
                c_gen_expr(cg, obj->value);
            }
            println(cg, ";");
            break;
        case OBJ_FUNCTION:
            c_gen_type(cg, obj->return_type, "");
            print(cg, " %s(", obj_callee);
            if(obj->args)
                c_gen_fn_arg_list(cg, obj->args);
            println(cg, ");");
            break;
        case OBJ_TYPEDEF:
            if(obj->data_type->kind != TY_STRUCT)
                break;
            c_gen_type(cg, obj->data_type, obj_callee);
            if(obj->data_type->kind == TY_ARR)
                c_gen_array_brackets(cg, obj->data_type);
            println(cg, ";");
            break;
        case OBJ_NAMESPACE:
            for(size_t i = 0; i < obj->objs->size; i++)
                c_gen_obj_decl(cg, obj->objs->items[i]);
            break;
        default:
            break;
    }
}

static void c_gen_va_list_init(CCodegenData_T* cg, ASTObj_T* fn)
{
    char* va_id = c_gen_identifier(cg, ((ASTObj_T*) fn->args->items[fn->args->size - 1])->id);
    println(cg, "va_list %s;", va_id);

    char* last_arg_id = c_gen_identifier(cg, ((ASTObj_T*)fn->args->items[fn->args->size - 2])->id);
    println(cg, "va_start(%s, %s);", va_id, last_arg_id);
}

static void c_gen_va_list_end(CCodegenData_T* cg, ASTIdentifier_T* id)
{
    char* va_id = c_gen_identifier(cg, id);
    println(cg, "va_end(%s);", va_id);
}

static void c_gen_obj(CCodegenData_T* cg, ASTObj_T* obj)
{
    if(obj->is_extern)
        return;

    switch(obj->kind)
    {
        case OBJ_FUNCTION:
            {
                c_gen_type(cg, obj->return_type, "");
                print(cg, " %s(", c_gen_identifier(cg, obj->id));

                bool has_va_list = false;

                if(obj->args)
                    has_va_list = c_gen_fn_arg_list(cg, obj->args);
                println(cg, "){");

                if(has_va_list)
                    c_gen_va_list_init(cg, obj);

                c_gen_stmt(cg, obj->body);

                if(has_va_list)
                    c_gen_va_list_end(cg, ((ASTObj_T*)obj->args->items[obj->args->size - 1])->id);

                println(cg, "}");
            } break;
        case OBJ_NAMESPACE:
            for(size_t i = 0; i < obj->objs->size; i++)
            {
                c_gen_obj(cg, obj->objs->items[i]);
            }
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
        case ND_LONG:
            print(cg, "%ldL", node->long_val);
            break;
        case ND_LLONG:
            print(cg, "%lldLL", node->llong_val);
            break;
        case ND_FLOAT:
            print(cg, "%f", node->float_val);
            break;
        case ND_DOUBLE:
            print(cg, "%lf", node->double_val);
            break;
        case ND_BOOL:
            print(cg, node->bool_val ? "true" : "false");
            break;
        case ND_CHAR:
            print(cg, "'%s'", node->str_val);
            break;
        case ND_NIL:
            print(cg, "NULL");
            break;
        case ND_STR:
            print(cg, "\"%s\"", node->str_val);
            break;
        case ND_ID: 
            print(cg, "%s", c_gen_identifier(cg, node->id));
            break;
        case ND_CALL:
            c_gen_expr(cg, node->expr);
            print(cg, "(");
            for(int i = 0; i < node->args->size; i++)
            {
                c_gen_expr(cg, node->args->items[i]);
                print(cg, i < node->args->size - 1 ? "," : "");
            }
            print(cg, ")");
            break;
        case ND_ASSIGN:
            c_gen_expr(cg, node->left);
            print(cg, "=");
            if(node->data_type)
            {
                print(cg, "(");
                c_gen_type(cg, node->data_type, "");
                print(cg, ")");
            }
            c_gen_expr(cg, node->right);
            break;
        case ND_ADD:
            c_gen_expr(cg, node->left);
            print(cg, "+");
            c_gen_expr(cg, node->right);
            break;
        case ND_SUB:
            c_gen_expr(cg, node->left);
            print(cg, "-");
            c_gen_expr(cg, node->right);
            break;  
        case ND_DIV:
            c_gen_expr(cg, node->left);
            print(cg, "/");
            c_gen_expr(cg, node->right);
            break;
        case ND_MUL:
            c_gen_expr(cg, node->left);
            print(cg, "*");
            c_gen_expr(cg, node->right);
            break;
        case ND_EQ:
            c_gen_expr(cg, node->left);
            print(cg, "==");
            c_gen_expr(cg, node->right);
            break;
        case ND_LT:
            c_gen_expr(cg, node->left);
            print(cg, "<");
            c_gen_expr(cg, node->right);
            break;
        case ND_LE:
            c_gen_expr(cg, node->left);
            print(cg, "<=");
            c_gen_expr(cg, node->right);
            break;
        case ND_GT:
            c_gen_expr(cg, node->left);
            print(cg, ">");
            c_gen_expr(cg, node->right);
            break;
        case ND_GE:
            c_gen_expr(cg, node->left);
            print(cg, ">=");
            c_gen_expr(cg, node->right);
            break;
        case ND_NE:
            c_gen_expr(cg, node->left);
            print(cg, "!=");
            c_gen_expr(cg, node->right);
            break;
        case ND_AND:
            c_gen_expr(cg, node->left);
            print(cg, "&&");
            c_gen_expr(cg, node->right);
            break;
        case ND_OR:
            c_gen_expr(cg, node->left);
            print(cg, "||");
            c_gen_expr(cg, node->right);
            break;
        case ND_LSHIFT:
            c_gen_expr(cg, node->left);
            print(cg, "<<");
            c_gen_expr(cg, node->right);
            break;
        case ND_RSHIFT:
            c_gen_expr(cg, node->left);
            print(cg, ">>");
            c_gen_expr(cg, node->right);
            break;
        case ND_XOR:
            c_gen_expr(cg, node->left);
            print(cg, "^");
            c_gen_expr(cg, node->right);
            break;
        case ND_BIT_OR:
            c_gen_expr(cg, node->left);
            print(cg, "|");
            c_gen_expr(cg, node->right);
            break;
        case ND_BIT_AND:
            c_gen_expr(cg, node->left);
            print(cg, "&");
            c_gen_expr(cg, node->right);
            break;
        case ND_MOD:
            c_gen_expr(cg, node->left);
            print(cg, "%%");
            c_gen_expr(cg, node->right);
            break;
        case ND_REF:
        case ND_DEREF:
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
        case ND_CLOSURE:
            print(cg, "(");
            c_gen_expr(cg, node->expr);
            print(cg, ")");
            break;
        case ND_MEMBER:
            c_gen_expr(cg, node->left);
            print(cg, node->is_ptr ? "->" : ".");
            c_gen_expr(cg, node->right);
            break;
        case ND_INDEX:
            c_gen_expr(cg, node->left);
            print(cg, "[");
            c_gen_expr(cg, node->expr);
            print(cg, "]");
            break;
        case ND_ARRAY:
            print(cg, "{");

            for(size_t i = 0; i < node->args->size; i++)
            {
                c_gen_expr(cg, node->args->items[i]);
                if(i < node->args->size -1)
                    print(cg, ",");
            }

            print(cg, "}");
            break;
        case ND_STRUCT:
            print(cg, "{");

            for(size_t i = 0; i < node->args->size; i++)
            {
                c_gen_expr(cg, node->args->items[i]);
                if(i < node->args->size -1)
                    print(cg, ",");
            }

            print(cg, "}");
            break;
        case ND_CAST:
            print(cg, "(");
            c_gen_type(cg, node->data_type, "");
            if(node->data_type->kind == TY_ARR)
                c_gen_array_brackets(cg, node->data_type);
            print(cg, ")");
            c_gen_expr(cg, node->left);
            break;
        case ND_SIZEOF:
            print(cg, "sizeof(");
            if(node->data_type)
                c_gen_type(cg, node->data_type, "");
            else
                c_gen_expr(cg, node->expr);
            print(cg, ")");
            break;
        case ND_LEN:
            print(cg, "len(");
            c_gen_expr(cg, node->expr);
            print(cg, ")");
            break;
        case ND_VA_ARG:
            print(cg, "va_arg(");
            c_gen_expr(cg, node->expr);
            print(cg, ",");
            c_gen_type(cg, node->data_type, "");
            print(cg, ")");
            break;
        case ND_LAMBDA:
            print(cg, c_gen_identifier(cg, node->id));
            break;
        default:
            throw_error(ERR_MISC, node->tok, "Expressions of type %d are currently not supported", node->kind);
    }
}

static void c_gen_local(CCodegenData_T* cg, ASTObj_T* obj)
{
    c_gen_type(cg, obj->data_type, "");
    print(cg, " %s", c_gen_identifier(cg, obj->id));
    if(obj->data_type->kind == TY_ARR)
                c_gen_array_brackets(cg, obj->data_type);
    println(cg, ";"); 
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
            print(cg, "while(1)");
            c_gen_stmt(cg, node->body);
            break;
        case ND_WHILE:
            print(cg, "while(");
            c_gen_expr(cg, node->condition);
            print(cg, ")");
            c_gen_stmt(cg, node->body);
            break;
        case ND_FOR:
            println(cg, "{");
            for(size_t i = 0; i < node->locals->size; i++)
                c_gen_local(cg, node->locals->items[i]);
            
            print(cg, "for(");
            if(node->init_stmt) 
            {    
                c_gen_stmt(cg, node->init_stmt);
            } 
            else
                print(cg, ";");

            if(node->condition)
                c_gen_expr(cg, node->condition);
            print(cg, ";");
            if(node->expr)
                c_gen_expr(cg, node->expr);
            print(cg, ")");

            c_gen_stmt(cg, node->body);
            
            println(cg, "}");
            break;
        case ND_EXPR_STMT:
            c_gen_expr(cg, node->expr);
            println(cg, ";");
            break;
        case ND_MATCH:  // Using the default switch statement is not ideal here, I should be using a custom statement-like function
            print(cg, "switch(");
            c_gen_expr(cg, node->condition);
            println(cg, ") {");

            for(size_t i = 0; i < node->cases->size; i++)
                c_gen_stmt(cg, node->cases->items[i]);

            println(cg, "}");
            break;
        case ND_CASE:
            print(cg, "case ");
            c_gen_expr(cg, node->condition);
            println(cg, ":");

            c_gen_stmt(cg, node->body);

            println(cg, "break;");
            break;
        case ND_BREAK:
            println(cg, "break;");
            break;
        case ND_CONTINUE: 
            println(cg, "continue;");
            break;
        case ND_NOOP:
            break;
        case ND_ASM:
            print(cg, "__asm__(");
            c_gen_expr(cg, node->expr);
            println(cg, ");");
            break;
        default:
            break;
    }
}

static void c_gen_lambda_fn(CCodegenData_T* cg, ASTNode_T* lambda)
{
    c_gen_type(cg, lambda->data_type, "");
    print(cg, " %s(", c_gen_identifier(cg, lambda->id));

    c_gen_fn_arg_list(cg, lambda->args);

    println(cg, "){");
    c_gen_stmt(cg, lambda->body);
    println(cg, "}");
}

static List_T* get_id_path(ASTIdentifier_T* id) {
    List_T* path = init_list(sizeof(struct AST_IDENTIFIER_STRUCT*));
    list_push(path, id);

    ASTIdentifier_T* outer = id;
    while(outer->outer)
    {
        outer = outer->outer;
        list_push(path, outer);
    }

    return path;
}

static void cat_id(char* callee, ASTIdentifier_T* id)
{
    strcat(callee, id->callee);
}

static char* c_gen_identifier(CCodegenData_T* cg, ASTIdentifier_T* id)
{
    if(id->outer == NULL)
        return id->callee;

    static const char* CSP_PREFIX_STR = "__csp_";

    List_T* path = get_id_path(id);

    size_t len = (BUFSIZ) * path->size + 1;
    char callee[len];
    memset(callee, '\0', sizeof callee);
    strcat(callee, CSP_PREFIX_STR);

    for(size_t i = path->size - 1; i > 0; i--)
    {
        cat_id(callee, path->items[i]);
        strcat(callee, "__csp_");
    }
    cat_id(callee, path->items[0]);

    free_list(path);

    char* new_c = calloc(strlen(callee) + 1, sizeof(char));
    sprintf(new_c, "%s", callee);

    ast_mem_add_ptr(new_c);
    return new_c;
}