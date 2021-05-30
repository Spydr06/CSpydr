#include "codegen.h"
#include <stdarg.h>
#include <stdio.h>

#define GP_MAX 6
#define FP_MAX 8

const char *argreg8[] = {"%dil", "%sil", "%dl", "%cl", "%r8b", "%r9b"};
const char *argreg16[] = {"%di", "%si", "%dx", "%cx", "%r8w", "%r9w"};
const char *argreg32[] = {"%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d"};
const char *argreg64[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

static void gen_expr(ASTExpr_T* expr);
static void gen_stmt(ASTStmt_T* stmt);

codeGenerator_T* init_generator(errorHandler_T* eh)
{
    codeGenerator_T* cg = malloc(sizeof(struct CODE_GENERATOR_STRUCT));

    cg->current_func = NULL;
    cg->depth = 0;
    cg->count = 1;
    cg->eh = 0;
    cg->output_file = NULL;

    return cg;
}

void free_generator(codeGenerator_T* cg)
{
    free(cg);
}

void generate_asm(codeGenerator_T* cg, ASTProgram_T* prog, const char* target)
{
    cg->output_file = fopen(target, "wb");
    if(cg->output_file == NULL)
    {
        LOG_ERROR_F("[ERROR] IO: could not open file '%s'\n", target);
        exit(1);
    }

    fclose(cg->output_file);
}

__attribute__((format(printf, 2, 3)))
static void print_asm(codeGenerator_T* cg, char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(cg->output_file, fmt, ap);
    va_end(ap);
    fprintf(cg->output_file, "\n");
}

static int count(codeGenerator_T* cg) 
{
    return cg->count++;
}

static int align_to(int n, int align) {
    return (n + align - 1) / align * align;
}

static void push(codeGenerator_T* cg) {
    print_asm(cg, "  push %%rax");
    cg->depth++;
}

static void pop(codeGenerator_T* cg, char *arg) {
    print_asm(cg, "  pop %s", arg);
    cg->depth--;
}

static void pushf(codeGenerator_T* cg) {
    print_asm(cg, "  sub $8, %%rsp");
    print_asm(cg, "  movsd %%xmm0, (%%rsp)");
    cg->depth++;
}

static void popf(codeGenerator_T* cg, int reg) {
    print_asm(cg, "  movsd (%%rsp), %%xmm%d", reg);
    print_asm(cg, "  add $8, %%rsp");
    cg->depth--;
}

static void gen_expr(ASTExpr_T* expr)
{

}

static void gen_stmt(ASTStmt_T* stmt)
{

}