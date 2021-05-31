#include "codegen.h"
#include "../ast/types.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define GP_MAX 6
#define FP_MAX 8

#define MAX(x, y) ((x) < (y) ? (y) : (x))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

const char *argreg8[] = {"%dil", "%sil", "%dl", "%cl", "%r8b", "%r9b"};
const char *argreg16[] = {"%di", "%si", "%dx", "%cx", "%r8w", "%r9w"};
const char *argreg32[] = {"%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d"};
const char *argreg64[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

static void gen_expr(CodeGen_T* cg, ASTExpr_T* expr);
static void gen_stmt(CodeGen_T* cg, ASTStmt_T* stmt);

static void emit_data(CodeGen_T* cg, ASTProgram_T* prog);
static void emit_text(CodeGen_T* cg, ASTProgram_T* prog);

__attribute__((format(printf, 2, 3)))
static void print_asm(CodeGen_T* cg, char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(cg->output_file, fmt, ap);
    va_end(ap);
    fprintf(cg->output_file, "\n");
}

static int count(CodeGen_T* cg) 
{
    return cg->count++;
}

static int align_to(int n, int align) {
    return (n + align - 1) / align * align;
}

static void push(CodeGen_T* cg) {
    print_asm(cg, "  push %%rax");
    cg->depth++;
}

static void pop(CodeGen_T* cg, char *arg) {
    print_asm(cg, "  pop %s", arg);
    cg->depth--;
}

static void pushf(CodeGen_T* cg) {
    print_asm(cg, "  sub $8, %%rsp");
    print_asm(cg, "  movsd %%xmm0, (%%rsp)");
    cg->depth++;
}

static void popf(CodeGen_T* cg, int reg) {
    print_asm(cg, "  movsd (%%rsp), %%xmm%d", reg);
    print_asm(cg, "  add $8, %%rsp");
    cg->depth--;
}

CodeGen_T* init_generator(errorHandler_T* eh)
{
    CodeGen_T* cg = malloc(sizeof(struct CODE_GENERATOR_STRUCT));

    cg->current_fn = NULL;
    cg->depth = 0;
    cg->count = 1;
    cg->eh = 0;
    cg->output_file = NULL;

    return cg;
}

void free_generator(CodeGen_T* cg)
{
    free(cg);
}

void generate_asm(CodeGen_T* cg, ASTProgram_T* prog, const char* target)
{
    const char* asm_file_tmp = "%s.asm";
    char* asm_file = calloc(strlen(asm_file_tmp) + strlen(target) + 1, sizeof(char));
    sprintf(asm_file, asm_file_tmp, target);

    cg->output_file = fopen(asm_file, "wb");
    if(cg->output_file == NULL)
    {
        LOG_ERROR_F("[ERROR] IO: could not open file '%s'\n", asm_file);
        exit(1);
    }

    for(int i = 0; i < prog->files->size; i++)
    {
        ASTFile_T* file = (ASTFile_T*) prog->files->items[i];
        print_asm(cg, "    .file %d \"%s\"", i + 1, file->filepath);
    }

    emit_data(cg, prog);
    emit_text(cg, prog);


    fclose(cg->output_file);
}

static void gen_expr(CodeGen_T* cg, ASTExpr_T* expr)
{

}

static void gen_stmt(CodeGen_T* cg, ASTStmt_T* stmt)
{

}

static void emit_data(CodeGen_T* cg, ASTProgram_T* prog)
{   
    for(int i = 0; i < prog->files->size; i++)
    {
        ASTFile_T* file = (ASTFile_T*) prog->files->items[i];
        
        for(int i = 0; i < file->globals->size; i++)
        {
            ASTGlobal_T* global = (ASTGlobal_T*) file->globals->items[i];
            print_asm(cg, "    .globl %s", global->name);

            int align = (global->type->type == AST_ARRAY && global->type->size >= 16) ? MAX(16, global->align) : global->align;
            print_asm(cg, "    .comm %s, %d, %d", global->name, global->type->size, align);

            if(global->value)
            {
                print_asm(cg, "    .data");
                print_asm(cg, "    .type %s, @object", global->name);
                print_asm(cg, "    .size %s, %d", global->name, global->type->size);
                print_asm(cg, "    .align %d", align);

                print_asm(cg, "%s:", global->name);

            }
        }
    }
}

static void emit_text(CodeGen_T* cg, ASTProgram_T* prog)
{
    for(int i = 0; i < prog->files->size; i++)
    {
        ASTFile_T* file = (ASTFile_T*) prog->files->items[i];

        for(int i = 0; i < file->functions->size; i++)
        {
            ASTFunction_T* fn = (ASTFunction_T*) file->functions->items[i];
            print_asm(cg, "    .globl %s", fn->name);

            print_asm(cg, "    .text");
            print_asm(cg, "    .type %s, @function", fn->name);
            print_asm(cg, "%s:", fn->name);

            cg->current_fn = fn;

            // prologue
            print_asm(cg, "    push %%rbp");
            print_asm(cg, "    mov %%rsp %%rbp");
            print_asm(cg, "    sub $%d, %%rsp", fn->stack_size);
            print_asm(cg, "    mov %%rsp, %d(%%rbp)", fn->alloca_bottom->offset);
        }
    }
}

void assign_lvar_offsets(ASTFunction_T* fn)
{
    int top = 16;
    int bottom = 0;

    int gp = 0, fp = 0;

    for(int i = 0; i < fn->args->size; i++)
    {
        ASTArgument_T* arg = (ASTArgument_T*) fn->args->items[i];

        switch(arg->dataType->type)
        {
            case AST_STRUCT:
                // TODO: get struct sizes
                break;

            case AST_F32:
            case AST_F64:
                if(fp++ < FP_MAX)
                    continue;
                break;
            case AST_F80:
                break;
            default:
                if(gp++ < GP_MAX)
                    continue;
        }

        top = align_to(top, 8);
        arg->offset = top;
        top += arg->dataType->size;
    }

    for(int i = 0; i < fn->body->stmts->size; i++)
    {
        if(((ASTStmt_T*) fn->body->stmts->items[i])->type != STMT_LET)
            continue;
        ASTLocal_T* local = (ASTLocal_T*) ((ASTStmt_T* )fn->body->stmts->items[i])->stmt;

        int align = (local->dataType->type == AST_ARRAY && local->dataType->size >= 16) ? MAX(16, local->align) : local->align;

        bottom += local->dataType->size;
        bottom = align_to(bottom, align);
        local->offset = -bottom;
    }

    fn->stack_size = align_to(bottom, 16);
}

void assign_alloca_size(ASTFunction_T* fn)
{
    ASTLocal_T* alloca_bottom = initASTLocal(initASTType(AST_POINTER, primitives[AST_I8], NULL, NULL, 0, 0), NULL, "__alloca_size__", 0, 0);
    fn->alloca_bottom = alloca_bottom;
    listPush(fn->body->stmts, initASTStmt(STMT_LET, alloca_bottom));
}