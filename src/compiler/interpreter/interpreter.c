#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ast/ast.h"
#include "ast/ast_iterator.h"
#include "ast/types.h"
#include "codegen/codegen_utils.h"
#include "config.h"
#include "error/error.h"
#include "hashmap.h"
#include "interpreter.h"
#include "interpreter/stack.h"
#include "io/log.h"
#include "list.h"
#include "value.h"
#include "context.h"

#define PTR_TYPE(ty) ((ty)->kind == TY_PTR || (ty)->kind == TY_VLA)

#define PREFIX_OP_CASE(csp_type, field, value, op)          \
    case csp_type:                                          \
        (value).value.field = (op ((value).value.field));   \
        break

#define INTEGER_PREFIX_OP_CASES(value, op)           \
    PREFIX_OP_CASE(TY_I8,  integer.i8,  value, op);  \
    PREFIX_OP_CASE(TY_I16, integer.i16, value, op);  \
    PREFIX_OP_CASE(TY_I32, integer.i32, value, op);  \
    PREFIX_OP_CASE(TY_I64, integer.i64, value, op)

#define UINTEGER_PREFIX_OP_CASES(value, op)          \
    PREFIX_OP_CASE(TY_U8,  uinteger.u8,  value, op); \
    PREFIX_OP_CASE(TY_U16, uinteger.u16, value, op); \
    PREFIX_OP_CASE(TY_U32, uinteger.u32, value, op); \
    PREFIX_OP_CASE(TY_U64, uinteger.u64, value, op)

#define INTLIKE_PREFIX_OP_CASES(value, op)          \
    INTEGER_PREFIX_OP_CASES(value, op);             \
    UINTEGER_PREFIX_OP_CASES(value, op);            \
    PREFIX_OP_CASE(TY_CHAR, character, value, op);  \
    PREFIX_OP_CASE(TY_PTR, character, value, op)

#define FP_PREFIX_OP_CASE(value, op)                \
    PREFIX_OP_CASE(TY_F32, flt.f32, value, op);     \
    PREFIX_OP_CASE(TY_F64, flt.f64, value, op);     \
    PREFIX_OP_CASE(TY_F80, flt.f80, value, op)

#define NUMERIC_PREFIX_OP_CASES(value, op)          \
    INTLIKE_PREFIX_OP_CASES(value, op);             \
    FP_PREFIX_OP_CASE(value, op)

#define INFIX_OP_CASE(csp_type, field, left_value, right_value, op)                             \
    case csp_type:                                                                              \
        (left_value).value.field = (((left_value).value.field) op ((right_value).value.field)); \
        break

#define INTEGER_INFIX_OP_CASES(left_value, right_value, op) \
    INFIX_OP_CASE(TY_I8,  integer.i8,  left_value, right_value, op);  \
    INFIX_OP_CASE(TY_I16, integer.i16, left_value, right_value, op);  \
    INFIX_OP_CASE(TY_I32, integer.i32, left_value, right_value, op);  \
    INFIX_OP_CASE(TY_I64, integer.i64, left_value, right_value, op)

#define UINTEGER_INFIX_OP_CASES(left_value, right_value, op)          \
    INFIX_OP_CASE(TY_U8,  uinteger.u8,  left_value, right_value, op); \
    INFIX_OP_CASE(TY_U16, uinteger.u16, left_value, right_value, op); \
    INFIX_OP_CASE(TY_U32, uinteger.u32, left_value, right_value, op); \
    INFIX_OP_CASE(TY_U64, uinteger.u64, left_value, right_value, op)

#define INTLIKE_INFIX_OP_CASES(left_value, right_value, op)         \
    INTEGER_INFIX_OP_CASES(left_value, right_value, op);            \
    UINTEGER_INFIX_OP_CASES(left_value, right_value, op);           \
    INFIX_OP_CASE(TY_CHAR, character, left_value, right_value, op); \
    INFIX_OP_CASE(TY_PTR, character, left_value, right_value, op)

#define FP_INFIX_OP_CASES(left_value, right_value, op)              \
    INFIX_OP_CASE(TY_F32, flt.f32, left_value, right_value, op);    \
    INFIX_OP_CASE(TY_F64, flt.f64, left_value, right_value, op);    \
    INFIX_OP_CASE(TY_F80, flt.f80, left_value, right_value, op)

#define NUMERIC_INFIX_OP_CASES(left_value, right_value, op)         \
    INTLIKE_INFIX_OP_CASES(left_value, right_value, op);            \
    FP_INFIX_OP_CASES(left_value, right_value, op)

#define COMPARISON_OP_CASE(node_kind, op) case node_kind: do {      \
    InterpreterValue_T left_value = eval_expr(ictx, expr->left);    \
    InterpreterValue_T right_value = eval_expr(ictx, expr->right);  \
    switch(left_value.type->kind) {                                 \
        case TY_F32: case TY_F64: case TY_F80:                      \
            return BOOL_VALUE(interpreter_value_f80(&left_value) op interpreter_value_f80(&right_value)); \
        default:                                                    \
            return BOOL_VALUE(interpreter_value_i64(&left_value) op interpreter_value_i64(&right_value)); \
    }} while(0)

#define INTEGER_INFIX_OP_CASE(node_kind, op) case node_kind: do {       \
        InterpreterValue_T left_value = eval_expr(ictx, expr->left);    \
        InterpreterValue_T right_value = eval_expr(ictx, expr->right);  \
        switch(left_value.type->kind) {                                 \
            INTLIKE_INFIX_OP_CASES(left_value, right_value, op);        \
            default: unreachable();                                     \
        }                                                               \
        return left_value;                                              \
    } while(0)

#define NO_CAST_INT 1
#define NO_CAST ((InterpreterValue_T (*const)(InterpreterContext_T*, InterpreterValue_T*)) NO_CAST_INT)

#define PRIMITIVE_CAST_FN_DECL(from_c_type, to_c_type, to_csp_type, from_field, to_field)               \
    static InterpreterValue_T from_c_type##to_c_type __attribute((maybe_unused)) (InterpreterContext_T* _, InterpreterValue_T* v) { \
        return (InterpreterValue_T) {                                                                   \
            .type = primitives[to_csp_type],                                                            \
            .value = {.to_field = ((to_c_type) v->value.from_field)}                                    \
        };                                                                                              \
    }

typedef i32 Enum;

#define PRIMITIVE_CAST_FN_DECLS(ty, field)                         \
    PRIMITIVE_CAST_FN_DECL(ty, i8,  TY_I8,  field, integer.i8 )    \
    PRIMITIVE_CAST_FN_DECL(ty, i16, TY_I16, field, integer.i16)    \
    PRIMITIVE_CAST_FN_DECL(ty, i32, TY_I32, field, integer.i32)    \
    PRIMITIVE_CAST_FN_DECL(ty, i64, TY_I64, field, integer.i64)    \
    PRIMITIVE_CAST_FN_DECL(ty, u8,  TY_U8,  field, uinteger.u8 )   \
    PRIMITIVE_CAST_FN_DECL(ty, u16, TY_U16, field, uinteger.u16)   \
    PRIMITIVE_CAST_FN_DECL(ty, u32, TY_U32, field, uinteger.u32)   \
    PRIMITIVE_CAST_FN_DECL(ty, u64, TY_U64, field, uinteger.u64)   \
    PRIMITIVE_CAST_FN_DECL(ty, f32, TY_F32, field, flt.f32)        \
    PRIMITIVE_CAST_FN_DECL(ty, f64, TY_F64, field, flt.f64)        \
    PRIMITIVE_CAST_FN_DECL(ty, f80, TY_F80, field, flt.f80)        \
    PRIMITIVE_CAST_FN_DECL(ty, bool, TY_BOOL, field, boolean)      \
    PRIMITIVE_CAST_FN_DECL(ty, char, TY_CHAR, field, character)    \
    PRIMITIVE_CAST_FN_DECL(ty, Enum, TY_ENUM, field, integer.i32)

#define PRIMITIVE_MAP_ENTRY(c_type) \
    c_type##i8, c_type##i16, c_type##i32, c_type##i64, c_type##u8, c_type##u16, c_type##u32, c_type##u64, c_type##f32, c_type##f64, c_type##f80, c_type##bool, [TY_CHAR]=c_type##char, [TY_ENUM]=c_type##Enum

PRIMITIVE_CAST_FN_DECLS(i8,  integer.i8 )
PRIMITIVE_CAST_FN_DECLS(i16, integer.i16)
PRIMITIVE_CAST_FN_DECLS(i32, integer.i32)
PRIMITIVE_CAST_FN_DECLS(i64, integer.i64)

PRIMITIVE_CAST_FN_DECLS(u8,  uinteger.u8 )
PRIMITIVE_CAST_FN_DECLS(u16, uinteger.u16)
PRIMITIVE_CAST_FN_DECLS(u32, uinteger.u32)
PRIMITIVE_CAST_FN_DECLS(u64, uinteger.u64)

PRIMITIVE_CAST_FN_DECLS(f32, flt.f32)
PRIMITIVE_CAST_FN_DECLS(f64, flt.f64)
PRIMITIVE_CAST_FN_DECLS(f80, flt.f80)

PRIMITIVE_CAST_FN_DECLS(bool, boolean)
PRIMITIVE_CAST_FN_DECLS(char, character)
PRIMITIVE_CAST_FN_DECLS(Enum, integer.i32)

static InterpreterValue_T (*const type_cast_map[TY_KIND_LEN][TY_KIND_LEN])(InterpreterContext_T*, InterpreterValue_T*) = {
    [TY_I8]  = { PRIMITIVE_MAP_ENTRY(i8)  },
    [TY_I16] = { PRIMITIVE_MAP_ENTRY(i16) },
    [TY_I32] = { PRIMITIVE_MAP_ENTRY(i32) },
    [TY_I64] = { PRIMITIVE_MAP_ENTRY(i64) },
    [TY_U8]  = { PRIMITIVE_MAP_ENTRY(u8)  },
    [TY_U16] = { PRIMITIVE_MAP_ENTRY(u16) },
    [TY_U32] = { PRIMITIVE_MAP_ENTRY(u32) },
    [TY_U64] = { PRIMITIVE_MAP_ENTRY(u64) },
    [TY_F32] = { PRIMITIVE_MAP_ENTRY(f32) },
    [TY_F64] = { PRIMITIVE_MAP_ENTRY(f64) },
    [TY_F80] = { PRIMITIVE_MAP_ENTRY(f80) },
    [TY_BOOL] = { PRIMITIVE_MAP_ENTRY(_Bool) },
    [TY_VOID] = {0},
    [TY_CHAR] = { PRIMITIVE_MAP_ENTRY(char) },
    [TY_ENUM] = { PRIMITIVE_MAP_ENTRY(Enum) }
};

static void assign_lvar_offsets(InterpreterContext_T* ictx);

static void eval_stmt(InterpreterContext_T* ictx, ASTNode_T* stmt);
static InterpreterValue_T eval_expr(InterpreterContext_T* ictx, ASTNode_T* expr);
static InterpreterValue_T call_fn(InterpreterContext_T* ictx, ASTObj_T* fn, const InterpreterValueList_T* args);

static void init_interpreter_context(InterpreterContext_T* ictx, Context_T* context, ASTProg_T* ast)
{
    ictx->context = context;
    ictx->ast = ast;
    ictx->stack = init_interpreter_stack(BUFSIZ);
    ictx->string_literals = hashmap_init();

    // assure that address 0 is always occupied
    u8 null_byte = 0;
    interpreter_stack_push(&ictx->stack, &null_byte, sizeof null_byte);
}

static void free_interpreter_context(InterpreterContext_T* ictx)
{
    free_interpreter_stack(ictx->stack);
    hashmap_free(ictx->string_literals);
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

    assign_lvar_offsets(&ictx);

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

static void collect_string_literal(ASTNode_T* str_lit, va_list args)
{
    InterpreterContext_T* ictx = va_arg(args, InterpreterContext_T*);
    if(!hashmap_get(ictx->string_literals, str_lit->str_val))
    {
        InterpreterValue_T addr = push_str_lit(ictx, str_lit->str_val);
        hashmap_put(ictx->string_literals, str_lit->str_val, (void*) addr.value.ptr);
    }
}

static void assign_lvar_offsets(InterpreterContext_T* ictx)
{
    static const ASTIteratorList_T iterator = {
        .node_start_fns = {
            [ND_STR] = collect_string_literal
        },
    };
    ast_iterate(&iterator, ictx->ast, ictx);
}

static void eval_stmt(InterpreterContext_T* ictx, ASTNode_T* stmt)
{
    switch(stmt->kind)
    {
        case ND_NOOP:
            break;
        case ND_RETURN:
            ictx->returned = true;
            ictx->return_value = stmt->return_val ? eval_expr(ictx, stmt->return_val) : VOID_VALUE;
            break;
        case ND_EXPR_STMT:
            eval_expr(ictx, stmt->expr);
            break;
        case ND_USING:
            if(stmt->body)
                eval_stmt(ictx, stmt->body);
            break;
        case ND_BLOCK:
            // TODO: register local variables
            for(size_t i = 0; i < stmt->stmts->size && !ictx->returned && !ictx->broken && !ictx->continued; i++)
                eval_stmt(ictx, stmt->stmts->items[i]);
            break;
        case ND_IF:
            if(interpreter_value_is_falsy(eval_expr(ictx, stmt->condition)))
                eval_stmt(ictx, stmt->else_branch);
            else
                eval_stmt(ictx, stmt->if_branch);
            break;
        case ND_DO_UNLESS:
            if(interpreter_value_is_falsy(eval_expr(ictx, stmt->condition)))
                eval_stmt(ictx, stmt->body);
            break;
        case ND_LOOP:
            ictx->broken = false;
            while(!ictx->broken && !ictx->returned)
            {
                ictx->continued = false;
                eval_stmt(ictx, stmt->body);
            };
            break;
        case ND_WHILE:
            ictx->broken = false;
            while(!ictx->broken && !ictx->returned && interpreter_value_is_truthy(eval_expr(ictx, stmt->condition)))
            {
                ictx->continued = false;
                eval_stmt(ictx, stmt->body);
            }
            break;
        case ND_DO_WHILE:
            ictx->broken = false;
            do {
                ictx->continued = false;
                eval_stmt(ictx, stmt->body);
            } while(!ictx->broken && !ictx->returned && interpreter_value_is_truthy(eval_expr(ictx, stmt->condition)));
            break;
        case ND_FOR:
            ictx->broken = false;
            if(stmt->init_stmt)
                eval_stmt(ictx, stmt->init_stmt);
            while(interpreter_value_is_truthy(eval_expr(ictx, stmt->condition)))
            {
                ictx->continued = false;
                eval_stmt(ictx, stmt->body);
                if(ictx->broken || ictx->returned)
                    break;
                eval_expr(ictx, stmt->expr);
            }
            break;
        // TODO: ND_FOR_RANGE
        case ND_CONTINUE:
            ictx->continued = true;
            break;
        case ND_BREAK:
            ictx->broken = true;
            break;
        case ND_MATCH:
        {
            InterpreterValue_T condition = eval_expr(ictx, stmt->condition);

            for(size_t i = 0; i < stmt->cases->size; i++)
            {
                ASTNode_T* case_stmt = stmt->cases->items[i];
                InterpreterValue_T case_condition = eval_expr(ictx, case_stmt->condition);
                if(interpreter_values_equal(condition, case_condition))
                {
                    eval_stmt(ictx, case_stmt->body);
                    return;
                }
            }

            if(stmt->default_case)
                eval_stmt(ictx, stmt->default_case->body);
            break;
        }
        case ND_MATCH_TYPE:
            if(stmt->body)
                eval_stmt(ictx, stmt->body);
            break;
        // TODO: ND_DEFER
        // TODO: ND_ASM
        // TODO: ND_WITH
        default:    
            throw_error(ictx->context, ERR_INTERNAL, stmt->tok, "interpreting this stmt is not implemented yet");
    }
}

static inline InterpreterValue_T eval_add(InterpreterContext_T* ictx, ASTNode_T* expr)
{
    InterpreterValue_T left_value = eval_expr(ictx, expr->left);
    InterpreterValue_T right_value = eval_expr(ictx, expr->right);
    
    ASTType_T* left_type = unpack(expr->left->data_type);
    if(PTR_TYPE(left_type)) {
        left_value.type = left_type;
        left_value.value.ptr += interpreter_value_i64(&right_value) * left_type->base->size;
        return left_value;
    }

    switch(left_value.type->kind)
    {
        NUMERIC_INFIX_OP_CASES(left_value, right_value, +);
        default:
            unreachable();
    }
    return left_value;
}

static inline InterpreterValue_T eval_sub(InterpreterContext_T* ictx, ASTNode_T* expr)
{
    InterpreterValue_T left_value = eval_expr(ictx, expr->left);
    InterpreterValue_T right_value = eval_expr(ictx, expr->right);
    
    ASTType_T* left_type = unpack(expr->left->data_type);
    ASTType_T* right_type = unpack(expr->right->data_type);
    if(PTR_TYPE(left_type) && PTR_TYPE(right_type))
    {
        left_value.type = primitives[TY_I64];
        left_value.value.integer.i64 = ((i64) left_value.value.ptr - (i64) right_value.value.ptr) / left_type->base->size;
        return left_value;
    }
    else if(PTR_TYPE(left_type)) {
        left_value.type = left_type;
        left_value.value.ptr -= interpreter_value_i64(&right_value) * left_type->base->size;
        return left_value;
    }

    switch(left_value.type->kind)
    {
        NUMERIC_INFIX_OP_CASES(left_value, right_value, -);
        default:
            unreachable();
    }
    return left_value;
}

static InterpreterValue_T eval_expr(InterpreterContext_T* ictx, ASTNode_T* expr)
{
    switch(expr->kind)
    {
        case ND_NOOP:
            return VOID_VALUE;
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
        // TODO: ND_STRING
        case ND_FLOAT:
            return F32_VALUE(expr->float_val);
        case ND_DOUBLE:
            return F64_VALUE(expr->double_val);
        case ND_NIL:
            return NIL_VALUE;
        case ND_CLOSURE:
        {
            InterpreterValue_T value;
            for(size_t i = 0; i < expr->exprs->size; i++)
                value = eval_expr(ictx, expr->exprs->items[i]);
            return value;
        }
        case ND_SIZEOF:
            return I64_VALUE(expr->the_type->size);
        case ND_ALIGNOF:
            return I64_VALUE(expr->the_type->align);
        case ND_PIPE:
            ictx->pipe_value = eval_expr(ictx, expr->left);
            return eval_expr(ictx, expr->right);
        case ND_HOLE:
            return ictx->pipe_value;    
        case ND_TERNARY:
            if(interpreter_value_is_falsy(eval_expr(ictx, expr->condition)))
                return eval_expr(ictx, expr->else_branch);
            return eval_expr(ictx, expr->if_branch);
        case ND_ELSE_EXPR:
        {
            InterpreterValue_T left = eval_expr(ictx, expr->left);
            if(interpreter_value_is_falsy(left))
                return eval_expr(ictx, expr->right);
            return left;
        }
        case ND_OR:
            return BOOL_VALUE(interpreter_value_is_truthy(eval_expr(ictx, expr->left)) || interpreter_value_is_truthy(eval_expr(ictx, expr->right)));
        case ND_AND:
            return BOOL_VALUE(interpreter_value_is_truthy(eval_expr(ictx, expr->left)) && interpreter_value_is_truthy(eval_expr(ictx, expr->right)));
        case ND_NEG:
        {
            InterpreterValue_T value = eval_expr(ictx, expr->right);
            switch(value.type->kind)
            {
                NUMERIC_PREFIX_OP_CASES(value, -);
                default: unreachable();
            }
            return value;
        }
        // TODO: ND_INDEX
        // TODO: ND_INC, ND_DEC
        // TODO: ND_LEN
        case ND_ADD:
            return eval_add(ictx, expr);
        case ND_SUB:
            return eval_sub(ictx, expr);
        case ND_MUL:
        {
            InterpreterValue_T left_value = eval_expr(ictx, expr->left);
            InterpreterValue_T right_value = eval_expr(ictx, expr->right);
            switch(left_value.type->kind)
            {
                NUMERIC_INFIX_OP_CASES(left_value, right_value, *);
                default: unreachable();
            }
            return left_value;
        }
        case ND_DIV:
        {
            InterpreterValue_T left_value = eval_expr(ictx, expr->left);
            InterpreterValue_T right_value = eval_expr(ictx, expr->right);
            switch(left_value.type->kind)
            {
                NUMERIC_INFIX_OP_CASES(left_value, right_value, /);
                default: unreachable();
            }
            return left_value;
        }
        INTEGER_INFIX_OP_CASE(ND_MOD, %);
        // TODO: ND_ID
        // TODO: ND_MEMBER
        // TODO: ND_ARRAY
        // TODO: ND_STRUCT
        // TODO: ND_REF
        // TODO: ND_DEREF
        // TODO: ND_ASSIGN
        // TODO: ND_LAMBDA
        case ND_CAST:
        {
            InterpreterValue_T value = eval_expr(ictx, expr->left);
            InterpreterValue_T (*const cast_fn)(InterpreterContext_T*, InterpreterValue_T*) = type_cast_map[value.type->kind][unpack(expr->data_type)->kind];
            switch((uintptr_t) cast_fn) {
                case 0: // no cast possible
                {
                    char* buf1 = malloc(BUFSIZ);
                    char* buf2 = malloc(BUFSIZ);

                    throw_error(ictx->context, ERR_INTERNAL, expr->tok, "(interpreter) cannot cast from `%s` to `%s`", 
                        ast_type_to_str(ictx->context, buf1, value.type, BUFSIZ),
                        ast_type_to_str(ictx->context, buf2, expr->data_type, BUFSIZ)
                    );
                    
                    free(buf1);
                    free(buf2);
                    return VOID_VALUE;
                }
                case NO_CAST_INT: // no cast needed
                    return value;
                default:
                    return cast_fn(ictx, &value);
            }
        }
        case ND_NOT:
            return BOOL_VALUE(interpreter_value_is_falsy(eval_expr(ictx, expr->right)));
        case ND_BIT_NEG:
        {
            InterpreterValue_T value = eval_expr(ictx, expr->right);
            switch(value.type->kind)
            {
                INTLIKE_PREFIX_OP_CASES(value, ~);
                default: unreachable();
            }
            return value;
        }
        INTEGER_INFIX_OP_CASE(ND_BIT_AND, &);
        INTEGER_INFIX_OP_CASE(ND_BIT_OR, |);
        INTEGER_INFIX_OP_CASE(ND_LSHIFT, <<);
        INTEGER_INFIX_OP_CASE(ND_RSHIFT, >>);
        INTEGER_INFIX_OP_CASE(ND_XOR, ^);
        case ND_EQ:
        {
            InterpreterValue_T left_value = eval_expr(ictx, expr->left);
            InterpreterValue_T right_value = eval_expr(ictx, expr->right);
            return BOOL_VALUE(interpreter_values_equal(left_value, right_value));
        }
        case ND_NE:
        {
            InterpreterValue_T left_value = eval_expr(ictx, expr->left);
            InterpreterValue_T right_value = eval_expr(ictx, expr->right);
            return BOOL_VALUE(!interpreter_values_equal(left_value, right_value));
        }
        COMPARISON_OP_CASE(ND_LT, <);
        COMPARISON_OP_CASE(ND_LE, <=);
        COMPARISON_OP_CASE(ND_GT, >);
        COMPARISON_OP_CASE(ND_GE, >=);
        // TODO: ND_CALL
        default:
            throw_error(ictx->context, ERR_INTERNAL, expr->tok, "interpreting this expr not implemented yet");
    }

    unreachable();
    return VOID_VALUE;
}

static InterpreterValue_T call_fn(InterpreterContext_T* ictx, ASTObj_T* fn, const InterpreterValueList_T* args)
{
    assert(args->size == fn->args->size);
    dump_stack(ictx->stack);
    
    ictx->returned = false;
    eval_stmt(ictx, fn->body);
    if(ictx->returned)
        return ictx->return_value;
    return VOID_VALUE;
}
