#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ast/types.h"
#include "config.h"
#include "error/error.h"
#include "interpreter.h"
#include "interpreter/stack.h"
#include "io/log.h"
#include "list.h"
#include "value.h"
#include "context.h"

static InterpreterValue_T call_fn(InterpreterContext_T* ictx, ASTObj_T* fn, const InterpreterValueList_T* args);
static InterpreterValue_T eval_stmt(InterpreterContext_T* ictx, ASTNode_T* stmt);
static InterpreterValue_T eval_expr(InterpreterContext_T* ictx, ASTNode_T* expr);

static void init_interpreter_context(InterpreterContext_T* ictx, Context_T* context, ASTProg_T* ast)
{
    ictx->context = context;
    ictx->ast = ast;
    ictx->stack = init_interpreter_stack(BUFSIZ);

    // assure that address 0 is always occupied
    u8 null_byte = 0;
    interpreter_stack_push(&ictx->stack, &null_byte, sizeof null_byte);
}

static void free_interpreter_context(InterpreterContext_T* ictx)
{
    free_interpreter_stack(ictx->stack);
}

static inline InterpreterValue_T push_str_lit(InterpreterContext_T* ictx, const char* str_lit)
{
    size_t stack_addr = interpreter_stack_push(&ictx->stack, str_lit, strlen(str_lit) + 1);
    return PTR_VALUE(stack_addr, char_ptr_type);
}

static inline InterpreterValue_T push_argv(InterpreterContext_T* ictx)
{
    InterpreterValue_T program_name_val = push_str_lit(ictx, ictx->ast->main_file_path);
    for(i32 i = 0; i < ictx->context->args.argc; i++)
        push_str_lit(ictx, ictx->context->args.argv[i]);
    InterpreterValue_T argv = {
        .type = &(ASTType_T){.kind = TY_PTR, .base = (ASTType_T*) char_ptr_type}, // &&char type
        .value = program_name_val.value // base addr
    };
    return argv;
}

i32 interpreter_pass(Context_T* context, ASTProg_T* ast)
{
    if(!context->flags.silent)
    {
        LOG_OK_F(COLOR_BOLD_BLUE "  Interpret " COLOR_RESET " %s%s", ast->main_file_path, context->args.argv && context->args.argc ? " [" : "\n");
        for(i32 i = 0; i < context->args.argc; i++)
            LOG_OK_F(COLOR_RESET "`%s`%s", context->args.argv[i], context->args.argc - i > 1 ? ", " : "]\n" COLOR_RESET);
    }

    InterpreterContext_T ictx;
    init_interpreter_context(&ictx, context, ast);

    if(!ast->entry_point)
    {
        LOG_ERROR(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " no entry point defined.\n" COLOR_RESET);
        return 1;
    }

    InterpreterValue_T return_value;
    switch(ast->mfk)
    {
        case MFK_NO_ARGS:
            return_value = call_fn(&ictx, ast->entry_point, &EMPTY_INTERPERTER_VALUE_LIST); 
            break;
        case MFK_ARGV_PTR:
        {
            const ConstInterpeterValueList_M(1) args = {
                .size = 1,
                .data = {push_argv(&ictx)}
            };
            return_value = call_fn(&ictx, ast->entry_point, (const InterpreterValueList_T*) &args);
        } break;
        case MFK_ARGC_ARGV_PTR:
        {
            const ConstInterpeterValueList_M(2) args = {
                .size = 2,
                .data = {I32_VALUE(context->args.argc), push_argv(&ictx)}
            };
            return_value = call_fn(&ictx, ast->entry_point, (const InterpreterValueList_T*) &args);
        } break;
        default:
            throw_error(context, ERR_INTERNAL, ast->entry_point->tok, "current entry point signature not implemented in the interpreter");
            return 1;
    }

    u8 exit_code = (u8) return_value.value.integer.i32;
    LOG_INFO_F("[%s terminated with exit code %s%d" COLOR_RESET "]\n", ast->main_file_path, exit_code ? COLOR_BOLD_RED : COLOR_BOLD_GREEN, (i32) exit_code);

    free_interpreter_context(&ictx);
    return (i32) exit_code;
}

static InterpreterValue_T call_fn(InterpreterContext_T* ictx, ASTObj_T* fn, const InterpreterValueList_T* args)
{
    assert(args->size == fn->args->size);
    dump_stack(ictx->stack);
    
    return eval_stmt(ictx, fn->body);
}

static InterpreterValue_T eval_stmt(InterpreterContext_T* ictx, ASTNode_T* stmt)
{
    switch(stmt->kind)
    {
        case ND_NOOP:
            break;
        case ND_RETURN:
            return eval_expr(ictx, stmt->return_val);
        default:
            throw_error(ictx->context, ERR_INTERNAL, stmt->tok, "interpreting this stmt is not implemented yet");
    }

    return VOID_VALUE;
}

static InterpreterValue_T eval_expr(InterpreterContext_T* ictx, ASTNode_T* expr)
{
    switch(expr->kind)
    {
        case ND_INT:
            return I32_VALUE(expr->int_val);
        case ND_LONG:
            return I64_VALUE(expr->long_val);
        case ND_ULONG:
            return U64_VALUE(expr->ulong_val);
        case ND_BOOL:
            return BOOL_VALUE(expr->bool_val);
        case ND_CHAR:
            return CHAR_VALUE(expr->int_val);
        case ND_FLOAT:
            return F32_VALUE(expr->float_val);
        case ND_DOUBLE:
            return F64_VALUE(expr->double_val);
        default:
            throw_error(ictx->context, ERR_INTERNAL, expr->tok, "interpreting this expr not implemented yet");
    }

    unreachable();
    return VOID_VALUE;
}
