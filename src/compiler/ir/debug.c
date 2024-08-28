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

void dbg_print_ir_type(IRType_T* type)
{
#define PRIMITIVE(ty, id) case IR_TYPE_##ty: \
        printf("%s", #id);                  \
        break

    switch(type->kind)
    {
        PRIMITIVE(I8, i8);
        PRIMITIVE(I16, i16);
        PRIMITIVE(I32, i32);
        PRIMITIVE(I64, i64);
        PRIMITIVE(U8,  u8);
        PRIMITIVE(U16, u16);
        PRIMITIVE(U32, u32);
        PRIMITIVE(U64, u64);
        PRIMITIVE(F32, f32);
        PRIMITIVE(F64, f64);
        PRIMITIVE(F80, f80);
        PRIMITIVE(VOID, void);
        
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

            if(i + 1 < type->fields->size)
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

            if(i + 1 < type->fn.arguments->size)
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

void dbg_print_ir_lvalue(IRLValue_T* lvalue) {
    switch(lvalue->kind) {
        case IR_LVALUE_ALLOCA:
            printf("alloca ");
            dbg_print_ir_type(lvalue->type);
            break;
        case IR_LVALUE_POP:
            printf("pop");
            break;
        case IR_LVALUE_GLOBAL:
            printf("global %s; ", lvalue->global_id);
            dbg_print_ir_type(lvalue->type);
        case IR_LVALUE_GLOBAL_PTR:
            printf("ptrof (global %s)", lvalue->global_id);
            break;
        case IR_LVALUE_FUNC_PTR:
            printf("ptrof (function %s)", lvalue->function_id);
    }
}

void dbg_print_ir_literal(IRLiteral_T* lit)
{
    switch(lit->kind)
    {
    case IR_LITERAL_VOID:
        printf("void");
        break;
    case IR_LITERAL_I8:
        printf("%hhdi8", lit->i8_lit);
        break;
    case IR_LITERAL_U8:
        printf("%hhuu8", lit->u8_lit);
        break;
    case IR_LITERAL_I32:
        printf("%di32", lit->i32_lit);
        break;
    case IR_LITERAL_I64:
        printf("%ldi64", lit->i64_lit);
        break;
    case IR_LITERAL_U64:
        printf("%luu64", lit->u64_lit);
        break;
    case IR_LITERAL_F32:
        printf("%ff32", lit->f32_lit);
        break;
    case IR_LITERAL_F64:
        printf("%ff64", lit->f64_lit);
        break;
    case IR_LITERAL_REG:
        printf("%%%u", lit->reg.id);
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
        printf("  ret ");
        dbg_print_ir_literal(&stmt->_return.lit);
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
        dbg_print_ir_literal(&stmt->goto_if.condition);
        printf(";\n");
        break;
    case IR_STMT_DECL:
        printf("  %%%u := ", stmt->decl.reg.id);
        dbg_print_ir_lvalue(&stmt->decl.value);
        printf(";\n");
        break;
    default:
        printf("<unknown stmt %d>\n", stmt->kind);
        unreachable();
    }
}

void dbg_print_ir_function(IRFunction_T* func, IRDebugFilter_T filter)
{
    if(func->is_extern && !(filter & IR_PRINT_EXTERN_FUNC))
        return;

    printf("%sfunction %s(", func->is_extern ? "extern " : "", func->mangled_id);
    for(size_t i = 0; i < func->params->size; i++)
    {
        IRParameter_T* arg = func->params->items[i];

        printf("%%%u: ", arg->reg.id);
        dbg_print_ir_type(arg->type);

        if(i + 1 < func->params->size)
            printf(", ");
    }

    if(func->variadic)
        printf(", ...");
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
        dbg_print_ir_literal(&global->value);
        printf(";\n");
    }
}

#endif
