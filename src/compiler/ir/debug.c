#include "debug.h"
#include "error/error.h"
#include "ir/ir.h"

#ifndef NDEBUG

#include <stdio.h>

void dbg_print_ir(IR_T* ir, IRDebugFilter_T filter)
{
    size_t i;
    if(filter & IR_PRINT_GLOBAL)
    {
        for(i = 0; i < ir->globals->size; i++)
            dbg_print_ir_global(ir->globals->items[i], filter);
    }

    if(filter & IR_PRINT_FUNC)
    {
        for(i = 0; i < ir->functions->size; i++)
            dbg_print_ir_function(ir->functions->items[i], filter);
    }
}

void dbg_print_ir_identifier(IRIdentifier_T* ident)
{
    switch(ident->kind)
    {
    case IR_ID_LOCAL:
        printf("_%u", ident->local_id);
        break;
    case IR_ID_PARAMETER:
        printf("_%u", ident->parameter_id);
        break;
    case IR_ID_GLOBAL:
    case IR_ID_FUNCTION:
        printf("%s", ident->mangled_id);
        break;
    default:
        unreachable();
    }
}

void dbg_print_ir_type(IRType_T* type)
{
#define PRIMITIVE(ty) case IR_TYPE_##ty:    \
        printf("%s", #ty);                  \
        break

    switch(type->kind)
    {
        PRIMITIVE(I8);
        PRIMITIVE(I16);
        PRIMITIVE(I32);
        PRIMITIVE(I64);
        PRIMITIVE(U8);
        PRIMITIVE(U16);
        PRIMITIVE(U32);
        PRIMITIVE(U64);
        PRIMITIVE(F32);
        PRIMITIVE(F64);
        PRIMITIVE(F80);
        PRIMITIVE(VOID);
        
    case IR_TYPE_C_ARRAY:
        dbg_print_ir_type(type->base);
        printf(" 'c[%lu]", type->indices);
        break;
    case IR_TYPE_ARRAY:
        dbg_print_ir_type(type->base);
        printf(" [%lu]", type->indices);
        break;
    case IR_TYPE_VLA:
        dbg_print_ir_type(type->base);
        printf(" []");
        break;
    case IR_TYPE_PTR:
        printf("pointer");
        break;
    case IR_TYPE_STRUCT:
    case IR_TYPE_UNION:
        printf("%s {", type->kind == IR_TYPE_STRUCT ? "struct" : "union");
        for(size_t i = 0; i < type->fields->size; i++)
        {
            IRTypeField_T* field = type->fields->items[i];
            printf(" %s: ", field->id);
            dbg_print_ir_type(field->type);
            printf(", ");
        }
        printf(" }");
        break;
    case IR_TYPE_FN:
        printf("fn<");
        dbg_print_ir_type(type->base);
        printf(">(");
        for(size_t i = 0; i < type->fn.arguments->size; i++)
        {
            IRType_T* arg_type = type->fn.arguments->items[i];
            dbg_print_ir_type(arg_type);
            printf(", ");
        }
        if(type->fn.variadic)
            printf("...)");
        else
            printf(")");
        break;
    default:
        unreachable();
    }
#undef PRIMITIVE
}

void dbg_print_ir_expr(IRExpr_T* expr)
{
    switch(expr->kind)
    {
    case IR_EXPR_I8_LIT:
        printf("%di8", (int) expr->i8_lit);
        break;
    case IR_EXPR_U8_LIT:
        printf("%du8", (int) expr->u8_lit);
        break;
    case IR_EXPR_I32_LIT:
        printf("%di32", expr->i32_lit);
        break;
    case IR_EXPR_I64_LIT:
        printf("%ldi64", expr->i64_lit);
        break;
    case IR_EXPR_U64_LIT:
        printf("%luu64", expr->u64_lit);
        break;
    case IR_EXPR_F32_LIT:
        printf("%ff32", expr->f32_lit);
        break;
    case IR_EXPR_F64_LIT:
        printf("%ff64", expr->f64_lit);
        break;
    case IR_EXPR_ARRAY_LIT:
        printf("[");
        for(size_t i = 0; i < expr->args->size; i++)
        {
            dbg_print_ir_expr(expr->args->items[i]);
            printf(", ");
        }
        printf("]");
        break;
    case IR_EXPR_CAST:
        dbg_print_ir_expr(expr->unary.expr);
        printf(": ");
        dbg_print_ir_type(expr->type);
        break;
    case IR_EXPR_ID:
        dbg_print_ir_identifier(&expr->ident);
        break;
    default:
        unreachable();
    }
}

void dbg_print_ir_stmt(IRStmt_T* stmt)
{
    switch(stmt->kind) 
    {
    case IR_STMT_RETURN:
        if(stmt->_return.value)
        {    
            printf("  ret ");
            dbg_print_ir_expr(stmt->_return.value);
        }
        else
            printf("  ret");
        printf(";\n");
        break;
    case IR_STMT_LABEL:
        printf("~%u:\n", stmt->label.id);
        break;
    case IR_STMT_GOTO:
        printf("  goto ~%u;\n", stmt->_goto.label_id);
        break;
    case IR_STMT_GOTO_IF:
        printf("  goto ~%u if ", stmt->goto_if.label_id);
        if(stmt->goto_if.negated)
            printf("!");
        dbg_print_ir_expr(stmt->goto_if.condition);
        printf(";\n");
        break;
    case IR_STMT_ASM:
        printf("  asm ");
        for(size_t i = 0; i < stmt->_asm.args->size; i++)
        {
            dbg_print_ir_expr(stmt->_asm.args->items[i]);
            printf(" ");
        }
        printf(";\n");
        break;
    case IR_STMT_PUSH_LOCAL:
        printf("  push _%u;\n", stmt->push_l.local->id);
        break;
    case IR_STMT_POP_LOCAL:
        printf("  pop;\n");
        break;
    case IR_STMT_ASSIGN:
        printf("  ");
        dbg_print_ir_expr(stmt->assign.dest);
        printf(" = ");
        dbg_print_ir_expr(stmt->assign.value);
        printf(";\n");
        break;
    default:
        unreachable();
    }
}

void dbg_print_ir_function(IRFunction_T* func, IRDebugFilter_T filter)
{
    if(func->is_extern && !(filter & IR_PRINT_EXTERN_FUNC))
        return;

    printf("%sfn %s(", func->is_extern ? "extern " : "", func->mangled_id);
    for(size_t i = 0; i < func->params->size; i++)
    {
        IRLocal_T* arg = func->params->items[i];
        printf("_%u ", arg->id);
        dbg_print_ir_type(arg->type);
        printf(", ");
    }

    if(func->variadic)
        printf("...");
    printf("): ");
    dbg_print_ir_type(func->return_type);

    if(func->is_extern)
    {
        printf(";\n");
        return;
    }

    printf(" {\n");
    for(size_t i = 0; i < func->stmts->size; i++)
        dbg_print_ir_stmt(func->stmts->items[i]);
    printf("}\n");
}

void dbg_print_ir_global(IRGlobal_T* global, IRDebugFilter_T filter)
{
    if(global->is_extern)
    {
        printf("extern global %s: ", global->mangled_id);
        dbg_print_ir_type(global->type);
        printf(";\n");
    }
    else
    {
        printf("global %s: ", global->mangled_id);
        dbg_print_ir_type(global->type);
        printf(" = ");
    fflush(stdout);
        if(global->value)
            dbg_print_ir_expr(global->value);
        else
            printf("undefined");
        printf(";\n");
    }
}

#endif
