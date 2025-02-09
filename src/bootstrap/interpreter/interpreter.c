#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ast/ast.h"
#include "ast/types.h"
#include "codegen/codegen_utils.h"
#include "config.h"
#include "error/error.h"
#include "hashmap.h"
#include "interpreter.h"
#include "interpreter/stack.h"
#include "io/log.h"
#include "lexer/token.h"
#include "list.h"
#include "value.h"
#include "context.h"

#define MAX_RECURSION_DEPTH 1024

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
    InterpreterValue_T left_value = interpreter_eval_expr(ictx, expr->left);    \
    InterpreterValue_T right_value = interpreter_eval_expr(ictx, expr->right);  \
    switch(left_value.type->kind) {                                 \
        case TY_F32: case TY_F64: case TY_F80:                      \
            return BOOL_VALUE(interpreter_value_f80(&left_value) op interpreter_value_f80(&right_value)); \
        default:                                                    \
            return BOOL_VALUE(interpreter_value_i64(&left_value) op interpreter_value_i64(&right_value)); \
    }} while(0)

#define INTEGER_INFIX_OP_CASE(node_kind, op) case node_kind: do {       \
        InterpreterValue_T left_value = interpreter_eval_expr(ictx, expr->left);    \
        InterpreterValue_T right_value = interpreter_eval_expr(ictx, expr->right);  \
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

PRIMITIVE_CAST_FN_DECLS(Ptr, ptr)

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
    [TY_ENUM] = { PRIMITIVE_MAP_ENTRY(Enum) },
    [TY_PTR]  = { PRIMITIVE_MAP_ENTRY(Ptr) }
};

static void eval_stmt(InterpreterContext_T* ictx, ASTNode_T* stmt);
static InterpreterValue_T call_fn(InterpreterContext_T* ictx, const ASTObj_T* fn, const InterpreterValueList_T* args);

static LValue_T lvalue_from_id(InterpreterContext_T* ictx, ASTObj_T* obj, ASTType_T* ty, Token_T* tok);
static InterpreterValue_T load_value(LValue_T lvalue);
static LValue_T eval_lvalue(InterpreterContext_T* ictx, ASTNode_T* node);
static void assign_lvalue(LValue_T dest, InterpreterValue_T* value);

void init_interpreter_context(InterpreterContext_T* ictx, Context_T* context, ASTProg_T* ast)
{
    ictx->context = context;
    ictx->ast = ast;
    ictx->stack = init_interpreter_stack(BUFSIZ);
    ictx->global_storage = init_interpreter_stack(BUFSIZ / 2); // a little smaller is usually enough
    ictx->recursion_depth = 0;
    ictx->constexpr_only = false;

    ictx->string_literals = hashmap_init();
    ictx->broken = false;

    // assure that address 0 is always occupied
    u8 null_byte = 0;
    interpreter_stack_push(&ictx->stack, &null_byte, sizeof null_byte);
}

void free_interpreter_context(InterpreterContext_T* ictx)
{
    free_interpreter_stack(ictx->stack);
    free_interpreter_stack(ictx->global_storage);
    hashmap_free(ictx->string_literals);
}

static inline InterpreterValue_T push_str_lit(InterpreterContext_T* ictx, const char* str_lit)
{
    size_t stack_addr = interpreter_stack_push(&ictx->global_storage, str_lit, strlen(str_lit) + 1);
    return PTR_VALUE(&ictx->global_storage[stack_addr], char_ptr_type);
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

    context->flags.run_after_compile = false;
    
    InterpreterContext_T ictx;
    init_interpreter_context(&ictx, context, ast);
    context->current_obj = NULL;
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
            const ConstInterpreterValueList_M(1) args = {
                .size = 1,
                .data = {push_argv(&ictx)}
            };
            return_value = call_fn(&ictx, ast->entry_point, (const InterpreterValueList_T*) &args);
        } break;
        case MFK_ARGC_ARGV_PTR:
        {
            const ConstInterpreterValueList_M(2) args = {
                .size = 2,
                .data = {I32_VALUE(context->args.argc), push_argv(&ictx)}
            };
            return_value = call_fn(&ictx, ast->entry_point, (const InterpreterValueList_T*) &args);
        } break;
        default:
            throw_error(context, ERR_INTERNAL, ast->entry_point->tok, "current entry point signature not implemented in the interpreter");
            return 1;
    }

    printf(COLOR_BOLD_CYAN ":: Global Storage:\n" COLOR_RESET);
    dump_stack(ictx.global_storage);

    u8 exit_code = (u8) return_value.value.integer.i32;
    LOG_INFO_F("[%s terminated with exit code %s%d" COLOR_RESET "]\n", ast->main_file_path, exit_code ? COLOR_BOLD_RED : COLOR_BOLD_GREEN, (i32) exit_code);

    free_interpreter_context(&ictx);
    return (i32) exit_code;
}

static void eval_stmt(InterpreterContext_T* ictx, ASTNode_T* stmt)
{
//    printf(">> stmt %d\n", stmt->kind);
    switch(stmt->kind)
    {
        case ND_NOOP:
            break;
        case ND_RETURN:
            ictx->returned = true;
            ictx->return_value = stmt->return_val ? interpreter_eval_expr(ictx, stmt->return_val) : VOID_VALUE;
            break;
        case ND_EXPR_STMT:
            interpreter_eval_expr(ictx, stmt->expr);
            break;
        case ND_USING:
            if(stmt->body)
                eval_stmt(ictx, stmt->body);
            break;
        case ND_BLOCK:
            for(size_t i = 0; i < stmt->stmts->size && !ictx->returned && !ictx->broken && !ictx->continued; i++)
                eval_stmt(ictx, stmt->stmts->items[i]);
        break;
        case ND_IF:
            if(interpreter_value_is_falsy(interpreter_eval_expr(ictx, stmt->condition)))
                eval_stmt(ictx, stmt->else_branch);
            else
                eval_stmt(ictx, stmt->if_branch);
            break;
        case ND_DO_UNLESS:
            if(interpreter_value_is_falsy(interpreter_eval_expr(ictx, stmt->condition)))
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
            while(!ictx->broken && !ictx->returned && interpreter_value_is_truthy(interpreter_eval_expr(ictx, stmt->condition)))
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
            } while(!ictx->broken && !ictx->returned && interpreter_value_is_truthy(interpreter_eval_expr(ictx, stmt->condition)));
            break;
        case ND_FOR:
            ictx->broken = false;
            if(stmt->init_stmt)
                eval_stmt(ictx, stmt->init_stmt);
            while(interpreter_value_is_truthy(interpreter_eval_expr(ictx, stmt->condition)))
            {
                ictx->continued = false;
                eval_stmt(ictx, stmt->body);
                if(ictx->broken || ictx->returned)
                    break;
                interpreter_eval_expr(ictx, stmt->expr);
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
            InterpreterValue_T condition = interpreter_eval_expr(ictx, stmt->condition);

            for(size_t i = 0; i < stmt->cases->size; i++)
            {
                ASTNode_T* case_stmt = stmt->cases->items[i];
                InterpreterValue_T case_condition = interpreter_eval_expr(ictx, case_stmt->condition);
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
    InterpreterValue_T left_value = interpreter_eval_expr(ictx, expr->left);
    InterpreterValue_T right_value = interpreter_eval_expr(ictx, expr->right);
    
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
    InterpreterValue_T left_value = interpreter_eval_expr(ictx, expr->left);
    InterpreterValue_T right_value = interpreter_eval_expr(ictx, expr->right);
    
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

static inline InterpreterValue_T cast_to_ptr(InterpreterContext_T* ictx, InterpreterValue_T* value, const ASTType_T* ptr_type, Token_T* tok)
{
#define CAST_IMPL_CASE(ty, field)                                                           \
    case ty:                                                                                \
        return (InterpreterValue_T) {                                                       \
            .type = ptr_type,                                                               \
            .value = (InterpreterValueUnion_T) { .ptr = (uintptr_t) (value->value.field) }  \
        }

    switch(value->type->kind)
    {
        CAST_IMPL_CASE(TY_I8, integer.i8);
        CAST_IMPL_CASE(TY_I16, integer.i16);
        CAST_IMPL_CASE(TY_I32, integer.i32);
        CAST_IMPL_CASE(TY_I64, integer.i64);
        CAST_IMPL_CASE(TY_U8, uinteger.u8);
        CAST_IMPL_CASE(TY_U16, uinteger.u16);
        CAST_IMPL_CASE(TY_U32, uinteger.u32);
        CAST_IMPL_CASE(TY_U64, uinteger.u64);
        CAST_IMPL_CASE(TY_F32, flt.f32);
        CAST_IMPL_CASE(TY_F64, flt.f64);
        CAST_IMPL_CASE(TY_F80, flt.f80);
        CAST_IMPL_CASE(TY_BOOL, boolean);
        CAST_IMPL_CASE(TY_CHAR, character);
        CAST_IMPL_CASE(TY_PTR, ptr);
        
        default:
        {
            char* buf1 = malloc(BUFSIZ);
            char* buf2 = malloc(BUFSIZ);
            *buf1 = *buf2 = '\0';

            throw_error(ictx->context, ERR_INTERNAL, tok, "(interpreter) cannot cast from `%s` to pointer type `%s`", 
                ast_type_to_str(ictx->context, buf1, value->type, BUFSIZ),
                ast_type_to_str(ictx->context, buf2, ptr_type, BUFSIZ)
            );
                    
            free(buf1);
            free(buf2);
            return VOID_VALUE; 
        }
    }
#undef CAST_IMPL
}

InterpreterValue_T interpreter_eval_expr(InterpreterContext_T* ictx, ASTNode_T* expr)
{
//    printf(">> expr %d\n", expr->kind);
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
        case ND_STR:
        {
            uintptr_t offset = (uintptr_t) hashmap_get(ictx->string_literals, expr->str_val);
            if(offset)
                return PTR_VALUE(offset, char_ptr_type);

            InterpreterValue_T addr = push_str_lit(ictx, expr->str_val);
            hashmap_put(ictx->string_literals, expr->str_val, (void*) addr.value.ptr);
            return addr;
        }
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
                value = interpreter_eval_expr(ictx, expr->exprs->items[i]);
            return value;
        }
        case ND_SIZEOF:
            return I64_VALUE(expr->the_type->size);
        case ND_ALIGNOF:
            return I64_VALUE(expr->the_type->align);
        case ND_PIPE:
            ictx->pipe_value = interpreter_eval_expr(ictx, expr->left);
            return interpreter_eval_expr(ictx, expr->right);
        case ND_HOLE:
            return ictx->pipe_value;    
        case ND_TERNARY:
            if(interpreter_value_is_falsy(interpreter_eval_expr(ictx, expr->condition)))
                return interpreter_eval_expr(ictx, expr->else_branch);
            return interpreter_eval_expr(ictx, expr->if_branch);
        case ND_ELSE_EXPR:
        {
            InterpreterValue_T left = interpreter_eval_expr(ictx, expr->left);
            if(interpreter_value_is_falsy(left))
                return interpreter_eval_expr(ictx, expr->right);
            return left;
        }
        case ND_OR:
            return BOOL_VALUE(interpreter_value_is_truthy(interpreter_eval_expr(ictx, expr->left)) || interpreter_value_is_truthy(interpreter_eval_expr(ictx, expr->right)));
        case ND_AND:
            return BOOL_VALUE(interpreter_value_is_truthy(interpreter_eval_expr(ictx, expr->left)) && interpreter_value_is_truthy(interpreter_eval_expr(ictx, expr->right)));
        case ND_NEG:
        {
            InterpreterValue_T value = interpreter_eval_expr(ictx, expr->right);
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
            InterpreterValue_T left_value = interpreter_eval_expr(ictx, expr->left);
            InterpreterValue_T right_value = interpreter_eval_expr(ictx, expr->right);
            switch(left_value.type->kind)
            {
                NUMERIC_INFIX_OP_CASES(left_value, right_value, *);
                default: unreachable();
            }
            return left_value;
        }
        case ND_DIV:
        {
            InterpreterValue_T left_value = interpreter_eval_expr(ictx, expr->left);
            InterpreterValue_T right_value = interpreter_eval_expr(ictx, expr->right);
            switch(left_value.type->kind)
            {
                NUMERIC_INFIX_OP_CASES(left_value, right_value, /);
                default: unreachable();
            }
            return left_value;
        }
        INTEGER_INFIX_OP_CASE(ND_MOD, %);
        case ND_ID:
        {
            ASTObj_T* obj = expr->referenced_obj;
            assert(obj != NULL);

            return load_value(lvalue_from_id(ictx, obj, expr->data_type, expr->tok));
        }
        // TODO: ND_MEMBER
        case ND_ARRAY:
        {
            
        }
        // TODO: ND_STRUCT
        case ND_REF:
        {
            LValue_T lvalue = eval_lvalue(ictx, expr->right);
            return PTR_VALUE(lvalue.ptr, expr->data_type);
        } 
        case ND_DEREF:
        {
            InterpreterValue_T value = interpreter_eval_expr(ictx, expr->right);
            return load_value((LValue_T) {
                .type = expr->data_type,
                .ptr = (void*) value.value.ptr
            });
        }
        case ND_ASSIGN:
        {
            LValue_T dest = eval_lvalue(ictx, expr->left);
            InterpreterValue_T value = interpreter_eval_expr(ictx, expr->right);
            assign_lvalue(dest, &value);
            return value;
        }

        // TODO: ND_LAMBDA
        case ND_CAST:
        {
            InterpreterValue_T value = interpreter_eval_expr(ictx, expr->left);
            const ASTType_T* to = unpack(expr->data_type), *from = value.type;

            if(to->kind == TY_PTR)
                return cast_to_ptr(ictx, &value, to, expr->tok);

            InterpreterValue_T (*const cast_fn)(InterpreterContext_T*, InterpreterValue_T*) = type_cast_map[from->kind][to->kind];
            switch((uintptr_t) cast_fn) {
                case 0: // no cast possible
                {
                    char* buf1 = malloc(BUFSIZ);
                    char* buf2 = malloc(BUFSIZ);
                    *buf1 = *buf2 = '\0';

                    throw_error(ictx->context, ERR_INTERNAL, expr->tok, "(interpreter) cannot cast from `%s` to `%s`", 
                        ast_type_to_str(ictx->context, buf1, from, BUFSIZ),
                        ast_type_to_str(ictx->context, buf2, to, BUFSIZ)
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
            return BOOL_VALUE(interpreter_value_is_falsy(interpreter_eval_expr(ictx, expr->right)));
        case ND_BIT_NEG:
        {
            InterpreterValue_T value = interpreter_eval_expr(ictx, expr->right);
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
            InterpreterValue_T left_value = interpreter_eval_expr(ictx, expr->left);
            InterpreterValue_T right_value = interpreter_eval_expr(ictx, expr->right);
            return BOOL_VALUE(interpreter_values_equal(left_value, right_value));
        }
        case ND_NE:
        {
            InterpreterValue_T left_value = interpreter_eval_expr(ictx, expr->left);
            InterpreterValue_T right_value = interpreter_eval_expr(ictx, expr->right);
            return BOOL_VALUE(!interpreter_values_equal(left_value, right_value));
        }
        COMPARISON_OP_CASE(ND_LT, <);
        COMPARISON_OP_CASE(ND_LE, <=);
        COMPARISON_OP_CASE(ND_GT, >);
        COMPARISON_OP_CASE(ND_GE, >=);
        case ND_CALL: {
            InterpreterValue_T callee = interpreter_eval_expr(ictx, expr->expr);
            InterpreterValueList_T* args = init_interpreter_value_list(expr->args->size);
            for(size_t i = 0; i < expr->args->size; i++)
            {
                InterpreterValue_T arg = interpreter_eval_expr(ictx, expr->args->items[i]);
                interpreter_value_list_push(&args, &arg);
            }

            InterpreterValue_T result = call_fn(ictx, callee.value.fn_obj, args);
            free_interpreter_value_list(args);
            return result;
        }
        default:
            throw_error(ictx->context, ERR_INTERNAL, expr->tok, "interpreting this expr not implemented yet");
    }

    unreachable();
    return VOID_VALUE;
}

static InterpreterValue_T call_fn(InterpreterContext_T* ictx, const ASTObj_T* fn, const InterpreterValueList_T* args)
{
    assert(args->size == fn->args->size);
    ictx->recursion_depth++;
    if(ictx->recursion_depth > MAX_RECURSION_DEPTH)
    {
        char* buf = malloc(BUFSIZ);
        *buf = '\0';
        throw_error(ictx->context, ERR_RECURSION_DEPTH, fn->tok, "attempt to call function `%s`, but recursion limit (%u) was reached", ast_id_to_str(buf, fn->id, BUFSIZ), MAX_RECURSION_DEPTH);
    }

    //    printf(COLOR_BOLD_CYAN ">>> Entering %s(%zu)\n" COLOR_RESET, fn->id->callee, args->size);
    
    const size_t stack_size_before = ictx->stack->size;
    for(size_t i = 0; i < args->size; i++) {
        const InterpreterValue_T* arg_value = &args->data[i];
        ASTObj_T* arg = fn->args->items[i];
        arg->offset = ictx->stack->size;
        interpreter_stack_push(&ictx->stack, &arg_value->value, arg->data_type->size);
    }

    size_t stack_size = ictx->stack->size;
    for(size_t i = 0; i < fn->objs->size; i++)
    {
        ASTObj_T* local = fn->objs->items[i];
        stack_size = align_to(stack_size, local->data_type->align);
        local->offset = stack_size;
        stack_size += local->data_type->size;
    }
    interpreter_stack_grow(&ictx->stack, stack_size - stack_size_before);

//    dump_stack(ictx->stack);
    
    eval_stmt(ictx, fn->body);

    interpreter_stack_shrink_to(ictx->stack, stack_size_before);
//    printf(COLOR_BOLD_CYAN "<<<\n" COLOR_RESET);
    ictx->recursion_depth--;

    if(ictx->returned) {
        ictx->returned = false;     
        return ictx->return_value;
    }
    return VOID_VALUE;
}

static LValue_T eval_lvalue(InterpreterContext_T* ictx, ASTNode_T* node) 
{
    switch(node->kind) {
        case ND_ID:
            assert(node->referenced_obj != NULL);
            return lvalue_from_id(ictx, node->referenced_obj, node->data_type, node->tok);
        case ND_DEREF:
            return (LValue_T) {
                .type = node->data_type,
                .ptr = (void*) interpreter_eval_expr(ictx, node->right).value.ptr
            };
        default:
            throw_error(ictx->context, ERR_INTERNAL, node->tok, "interpreting this lvalue not implemented yet");
    }
}

static void push_global(InterpreterContext_T* ictx, ASTObj_T* global)
{
    global->offset = interpreter_stack_align_to(&ictx->global_storage, global->data_type->align);
    if(global->value) 
    {
        InterpreterValue_T value = interpreter_eval_expr(ictx, global->value);
        interpreter_stack_push(&ictx->global_storage, &value.value, MIN(global->data_type->size, value.type->size));
    }
    else
        interpreter_stack_grow(&ictx->global_storage, global->data_type->size);
}

static LValue_T lvalue_from_id(InterpreterContext_T* ictx, ASTObj_T* obj, ASTType_T* ty, Token_T* tok)
{
    if(ictx->constexpr_only && !obj->constexpr && (obj->kind == OBJ_GLOBAL || obj->kind == OBJ_FUNCTION || obj->kind == OBJ_ENUM_MEMBER)) {
        char* buf = malloc(BUFSIZ);
        *buf = '\0';
        throw_error(ictx->context, ERR_CONSTEXPR, tok, "%s `%s` is not marked as `constexpr`", obj_kind_to_str(obj->kind), ast_id_to_str(buf, obj->id, BUFSIZ));
        free(buf);
    }
    
    switch(obj->kind) {
        case OBJ_LOCAL: 
        case OBJ_FN_ARG:
        {
            assert(obj->offset != 0);
            return (LValue_T) {
                .ptr = &ictx->stack->data[obj->offset],
                .type = ty
            };
        } break;
        case OBJ_FUNCTION:
            return (LValue_T) {
                .ptr = obj,
                .type = ty
            };
        break;
        case OBJ_GLOBAL: 
        case OBJ_ENUM_MEMBER:
        {
            if(obj->offset == 0)
                push_global(ictx, obj);
            return (LValue_T) {
                .ptr = &ictx->global_storage->data[obj->offset],
                .type = ty
            };
        }
        default:
            fprintf(stderr, "not implemented: load %s.\n", obj_kind_to_str(obj->kind));
            unreachable();
    }
}

static InterpreterValue_T load_value(LValue_T lvalue)
{ 
    switch(lvalue.type->kind) {
        case TY_I8:
            return INT_VALUE(TY_I8, i8, *(i8*) lvalue.ptr);
        case TY_I16:
            return INT_VALUE(TY_I16, i16, *(i16*) lvalue.ptr);
        case TY_I32:
            return INT_VALUE(TY_I32, i32, *(i32*) lvalue.ptr);
        case TY_I64:
            return INT_VALUE(TY_I64, i64, *(i64*) lvalue.ptr);
        case TY_U8:
            return UINT_VALUE(TY_U8, u8, *(u8*) lvalue.ptr);
        case TY_U16:
            return UINT_VALUE(TY_U16, u16, *(u16*) lvalue.ptr);
        case TY_U32:
            return UINT_VALUE(TY_U32, u32, *(u32*) lvalue.ptr);
        case TY_U64:
            return UINT_VALUE(TY_U64, u64, *(u64*) lvalue.ptr);
        case TY_F32:
            return FLOAT_VALUE(TY_F32, f32, *(f32*) lvalue.ptr);
        case TY_F64:
            return FLOAT_VALUE(TY_F64, f64, *(f64*) lvalue.ptr);
        case TY_F80:
            return FLOAT_VALUE(TY_F80, f80, *(f80*) lvalue.ptr);
        case TY_BOOL:
            return BOOL_VALUE(*(bool*) lvalue.ptr);
        case TY_CHAR:
            return CHAR_VALUE(*(char*) lvalue.ptr);
        case TY_VOID:
            return VOID_VALUE;
        case TY_PTR:
            return PTR_VALUE(*(void**) lvalue.ptr, lvalue.type->base);
        case TY_FN:
            return (InterpreterValue_T){.type = lvalue.type, .value = (InterpreterValueUnion_T){.fn_obj = lvalue.ptr} };
        default:
            fprintf(stderr, "not implemented: load type %s.\n", type_kind_to_str(lvalue.type->kind));
            unreachable();
    }
}

static void assign_lvalue(LValue_T dest, InterpreterValue_T* value) {
    assert(dest.type->size == value->type->size);
    memcpy(dest.ptr, &value->value, dest.type->size);
}

