#include "normalizer.h"
#include "ast/ast.h"
#include "ast/ast_iterator.h"
#include "codegen/codegen_utils.h"
#include "config.h"
#include "context.h"
#include "error/error.h"
#include "ir/ir.h"
#include "list.h"
#include "parser/typechecker.h"
#include "util.h"

#include <string.h>
#include <assert.h>
#include <stdarg.h>

#define GET_NORMALIZER(va) Normalizer_T* n = va_arg(va, Normalizer_T*)

typedef struct TYPE_PAIR_STRUCT {
    ASTType_T* ast_type;
    IRType_T* ir_type;
} TypePair_T;

static TypePair_T* new_type_pair(ASTType_T* ast_type, IRType_T* ir_type)
{
    TypePair_T* pair = malloc(sizeof(struct TYPE_PAIR_STRUCT));
    pair->ast_type = ast_type;
    pair->ir_type = ir_type;
    return pair;
}

static IRType_T* get_ir_type(Normalizer_T* n, ASTType_T* ast_type)
{
    size_t i;
    for(i = 0; i < n->types->size; i++)
    {
        TypePair_T* pair = n->types->items[i];
        if(pair->ast_type == ast_type)
            return pair->ir_type;
    }
    
    for(i = 0; i < n->types->size; i++)
    {
        TypePair_T* pair = n->types->items[i];
        if(types_equal(n->context, pair->ast_type, ast_type))
            return pair->ir_type;
    }

    return NULL;
}

static void normalize_function(ASTObj_T* func, va_list args);
static void normalize_global(ASTObj_T* global, va_list args);
static void normalize_enum_member(ASTObj_T* member, va_list args);

static LoopContext_T push_loop_context(Normalizer_T* n, LoopContext_T context)
{
    LoopContext_T old = n->loop_context;
    n->loop_context = context;
    return old;
}

int normalization_pass(Context_T* context, ASTProg_T* ast, IR_T* ir)
{
    ir_init(ir, context);

    Normalizer_T normalizer = {
        .context = context,
        .ir = ir,
        .types = init_list()
    };

    context->current_obj = NULL;
    static const ASTIteratorList_T iter = {
        .obj_start_fns = {
            [OBJ_FUNCTION] = normalize_function,
            [OBJ_GLOBAL] = normalize_global,
            [OBJ_ENUM_MEMBER] = normalize_enum_member,
        },
        .iterate_only_objs = true
    };

    ast_iterate(&iter, ast, &normalizer);

 //   free_list(ir->types);

    return 0;
}

static IRTypeKind_T translate_type_kind(ASTType_T* type)
{
#define PRIMITIVE(ty) [TY_##ty] = IR_TYPE_##ty

    assert(
        type->kind != TY_UNDEF && 
        type->kind != TY_TYPEOF && 
        type->kind != TY_TEMPLATE && 
        type->kind != TY_INTERFACE
    );

    static const IRTypeKind_T kinds[] = {
        PRIMITIVE(I8),
        PRIMITIVE(I16),
        PRIMITIVE(I32),
        PRIMITIVE(I64),
        PRIMITIVE(U8),
        PRIMITIVE(U16),
        PRIMITIVE(U32),
        PRIMITIVE(U64),
        PRIMITIVE(F32),
        PRIMITIVE(F64),
        PRIMITIVE(F80),
        PRIMITIVE(VOID),
        [TY_BOOL] = IR_TYPE_U8,
        [TY_CHAR] = IR_TYPE_I8,
        [TY_ENUM] = IR_TYPE_I32,
        PRIMITIVE(PTR),
        PRIMITIVE(ARRAY),
        PRIMITIVE(C_ARRAY),
        PRIMITIVE(STRUCT),
        PRIMITIVE(FN)
    };

    IRTypeKind_T ir_kind = kinds[type->kind];
    if(ir_kind == IR_TYPE_STRUCT && type->is_union)
        ir_kind = IR_TYPE_UNION;

    return ir_kind;

#undef PRIMITIVE
}

static IRType_T* normalize_type(Normalizer_T* n, ASTType_T* type)
{
    type = unpack(type);

    IRType_T* ir_type = get_ir_type(n, type);
    if(ir_type)
        return ir_type;

    ir_type = malloc(sizeof(struct IR_TYPE_STRUCT));
    list_push(n->types, new_type_pair(type, ir_type));

    CONTEXT_ALLOC_REGISTER(n->context, (void*) ir_type);

    ir_type->kind = translate_type_kind(type);
    switch(ir_type->kind)
    {
    case IR_TYPE_PTR:
    case IR_TYPE_VLA:
        ir_type->base = normalize_type(n, type->base);
        break;
    case IR_TYPE_C_ARRAY:
    case IR_TYPE_ARRAY:
        ir_type->base = normalize_type(n, type->base);
        ir_type->indices = type->num_indices;
        break;
    case IR_TYPE_FN:
        ir_type->base = normalize_type(n, type->base);
        ir_type->fn.variadic = type->is_variadic;

        ir_type->fn.arguments = init_list_sized(type->arg_types->size);
        CONTEXT_ALLOC_REGISTER(n->context, ir_type->fn.arguments);

        for(size_t i = 0; i < type->arg_types->size; i++)
            list_push(ir_type->fn.arguments, normalize_type(n, type->arg_types->items[i]));
        break;
    case IR_TYPE_STRUCT:
    case IR_TYPE_UNION:
        ir_type->fields = init_list_sized(type->members->size);
        CONTEXT_ALLOC_REGISTER(n->context, ir_type->fields);

        for(size_t i = 0; i < type->members->size; i++)
        {
            ASTNode_T* member = type->members->items[i];
            
            IRTypeField_T* field = malloc(sizeof(struct IR_TYPE_FIELD_STRUCT));
            CONTEXT_ALLOC_REGISTER(n->context, (void*) field);

            field->id = member->id->callee;
            field->offset = member->offset;
            field->type = normalize_type(n, member->data_type);
        }
        break;
    default:
        break;
    }

    return ir_type;
}

static IRExpr_T* normalize_basic_expr(Normalizer_T* n, ASTNode_T* expr)
{
    IRExpr_T* ir_expr = init_ir_expr(n->context, -1, normalize_type(n, expr->data_type));
    switch(expr->kind)
    {
    case ND_INT:
        ir_expr->kind = IR_EXPR_I32_LIT;
        ir_expr->i32_lit = expr->int_val;
        break;
    
    case ND_LONG:
        ir_expr->kind = IR_EXPR_I64_LIT;
        ir_expr->i64_lit = expr->long_val;
        break;

    case ND_ULONG:
        ir_expr->kind = IR_EXPR_U64_LIT;
        ir_expr->u64_lit = expr->ulong_val;
        break;

    case ND_FLOAT:
        ir_expr->kind = IR_EXPR_F32_LIT;
        ir_expr->f32_lit = expr->float_val;
        break;

    case ND_DOUBLE:
        ir_expr->kind = IR_EXPR_F64_LIT;
        ir_expr->f64_lit = expr->double_val;
        break;

    case ND_CHAR:
        ir_expr->kind = IR_EXPR_I8_LIT;
        ir_expr->i8_lit = (i8) expr->int_val;
        break;

    case ND_BOOL:
        ir_expr->kind = IR_EXPR_U8_LIT;
        ir_expr->u8_lit = (u8) expr->bool_val;
        break;

    case ND_ARRAY:
        ir_expr->kind = IR_EXPR_ARRAY_LIT;
        ir_expr->args = init_list_sized(expr->args->size);
        CONTEXT_ALLOC_REGISTER(n->context, ir_expr->args);
        for(size_t i = 0; i < expr->args->size; i++)
            list_push(ir_expr->args, normalize_basic_expr(n, expr->args->items[i]));
        break;
    
    case ND_CAST:
        ir_expr->kind = IR_EXPR_CAST;
        ir_expr->unary.expr = normalize_basic_expr(n, expr->left);
        break;

    default:
        throw_error(n->context, ERR_INTERNAL, expr->tok, "unexpected expr in normalization process");
    }

    assert(((i32) ir_expr->kind) != -1);
    return ir_expr;
}

static u64 make_label(Normalizer_T* n, IRFunction_T* ir_func)
{
    IRStmt_T* label = init_ir_stmt(n->context, IR_STMT_LABEL);
    label->label.id = n->label_id++;

    list_push(ir_func->stmts, label);
    return label->label.id;
}

static void make_goto(Normalizer_T* n, IRFunction_T* ir_func, u64 label_id, IRExpr_T* cond, bool negate)
{
    IRStmt_T* _goto = init_ir_stmt(n->context, cond ? IR_STMT_GOTO_IF : IR_STMT_GOTO);

    if(cond)
    {
        _goto->goto_if.label_id = label_id;
        _goto->goto_if.condition = cond;
        _goto->goto_if.negated = negate;
    }
    else
        _goto->_goto.label_id = label_id;

    list_push(ir_func->stmts, _goto);
}

static IRExpr_T* make_id_expr(Normalizer_T* n, IRIdentifier_T ident, IRType_T* type)
{
    IRExpr_T* expr = init_ir_expr(n->context, IR_EXPR_ID, type);
    expr->ident = ident;
    return expr;
}

static IRLocal_T* push_temp_local(Normalizer_T* n, IRType_T* type, IRExpr_T* init_value, IRFunction_T* ir_func)
{
    IRLocal_T* local = init_ir_local(n->context, n->locals_info.local_id++, type);
    local->temporary = true;
    
    IRStmt_T* stmt = init_ir_stmt(n->context, IR_STMT_PUSH_LOCAL);
    stmt->push_l.local = local;
    list_push(ir_func->stmts, stmt);

    if(init_value)
    {
        IRStmt_T* assign = init_ir_stmt(n->context, IR_STMT_ASSIGN);
        assign->assign.dest = make_id_expr(n, IR_LOCAL_ID(local->id), type);
        assign->assign.value = init_value;
        list_push(ir_func->stmts, assign);
    }

    return local;
}

static void pop_temp_local(Normalizer_T* n, IRFunction_T* ir_func)
{
    IRStmt_T* stmt = init_ir_stmt(n->context, IR_STMT_POP_LOCAL);
    list_push(ir_func->stmts, stmt);
}

static IRExpr_T* normalize_expr(Normalizer_T* n, ASTNode_T* expr, IRFunction_T* ir_func)
{
    switch(expr->kind)
    {
    default:
        return normalize_basic_expr(n, expr);
    }
}

static bool is_terminating(ASTNodeKind_T kind)
{
    return kind == ND_RETURN || kind == ND_CONTINUE || kind == ND_BREAK;
}

static void normalize_stmt(Normalizer_T* n, ASTNode_T* stmt, IRFunction_T* ir_func)
{
    IRStmt_T* ir_stmt = NULL;

    switch(stmt->kind)
    {
    case ND_NOOP:
        break;
    case ND_BLOCK:
        // TODO: init vars to zero
        for(size_t i = 0; i < stmt->stmts->size; i++)
        {
            normalize_stmt(n, stmt->stmts->items[i], ir_func);
            if(is_terminating(stmt->kind))
                break;
        }
        break;
    case ND_RETURN:
        ir_stmt = init_ir_stmt(n->context, IR_STMT_RETURN);
        if(stmt->return_val)
            ir_stmt->_return.value = normalize_expr(n, stmt->return_val, ir_func);
        break;
    case ND_BREAK:
        ir_stmt = init_ir_stmt(n->context, IR_STMT_GOTO);
        ir_stmt->_goto.label_id = n->loop_context.break_label;
        break;
    case ND_CONTINUE:
        ir_stmt = init_ir_stmt(n->context, IR_STMT_GOTO);
        ir_stmt->_goto.label_id = n->loop_context.continue_label;
        break;
    case ND_LOOP:
        {
            u64 continue_label = make_label(n, ir_func);            
            u64 break_label = n->label_id++;

            LoopContext_T outer = push_loop_context(n, (LoopContext_T){
                .break_label = break_label,
                .continue_label = continue_label
            });

            normalize_stmt(n, stmt->body, ir_func);
            make_goto(n, ir_func, continue_label, NULL, false);

            ir_stmt = init_ir_stmt(n->context, IR_STMT_LABEL);
            ir_stmt->label.id = break_label;

            push_loop_context(n, outer);
        } break;
    case ND_WHILE:
        {
            u64 continue_label = make_label(n, ir_func);
            u64 break_label = n->label_id++;

            LoopContext_T outer  = push_loop_context(n, (LoopContext_T){
                .break_label = break_label,
                .continue_label = continue_label
            });

            make_goto(n, ir_func, break_label, normalize_expr(n, stmt->condition, ir_func), true);
            normalize_stmt(n, stmt->body, ir_func);
            make_goto(n, ir_func, continue_label, NULL, false);
            
            ir_stmt = init_ir_stmt(n->context, IR_STMT_LABEL);
            ir_stmt->label.id = break_label;

            push_loop_context(n, outer);
        } break;
    case ND_DO_WHILE:
        {
            u64 begin_label = make_label(n, ir_func);
            u64 continue_label = n->label_id++;
            u64 break_label = n->label_id++;

            LoopContext_T outer = push_loop_context(n, (LoopContext_T) {
                .break_label = break_label,
                .continue_label = continue_label
            });

            normalize_stmt(n, stmt->body, ir_func);

            IRStmt_T* continue_label_stmt = init_ir_stmt(n->context, IR_STMT_LABEL);
            continue_label_stmt->label.id = continue_label;
            list_push(ir_func->stmts, continue_label_stmt);

            make_goto(n, ir_func, begin_label, normalize_expr(n, stmt->condition, ir_func), false);
            
            ir_stmt = init_ir_stmt(n->context, IR_STMT_LABEL);
            ir_stmt->label.id = break_label;

            push_loop_context(n, outer);
        } break;
    case ND_IF:
        {
            u64 else_label = n->label_id++;
            u64 end_label = stmt->else_branch ? n->label_id++ : else_label;
            
            make_goto(n, ir_func, else_label, normalize_expr(n, stmt->condition, ir_func), true);
            normalize_stmt(n, stmt->if_branch, ir_func);

            if(stmt->else_branch)
            {
                make_goto(n, ir_func, end_label, NULL, false);

                IRStmt_T* else_label_stmt = init_ir_stmt(n->context, IR_STMT_LABEL);
                else_label_stmt->label.id = else_label;
                list_push(ir_func->stmts, else_label_stmt);

                normalize_stmt(n, stmt->else_branch, ir_func);
            }

            ir_stmt = init_ir_stmt(n->context, IR_STMT_LABEL);
            ir_stmt->label.id = end_label;
        } break;
    case ND_DO_UNLESS:
        {
            u64 skip_label = n->label_id++;

            make_goto(n, ir_func, skip_label, normalize_expr(n, stmt->condition, ir_func), false);
            normalize_stmt(n, stmt->body, ir_func);

            ir_stmt = init_ir_stmt(n->context, IR_STMT_LABEL);
            ir_stmt->label.id = skip_label;
        } break;
    // ND_WITH
    // ND_FOR
    // ND_FOR_RANGE
    case ND_CASE:
        {
            IRStmt_T* label = init_ir_stmt(n->context, IR_STMT_LABEL); 
            label->label.id = stmt->ulong_val; 
            list_push(ir_func->stmts, label);
            
            normalize_stmt(n, stmt->body, ir_func);
            make_goto(n, ir_func, n->label_id, NULL, false);
        } break;
    case ND_MATCH:
        {
            IRExpr_T* value = normalize_expr(n, stmt->condition, ir_func);
            IRLocal_T* cmp = push_temp_local(n, value->type, value, ir_func);


            pop_temp_local(n, ir_func);
        } break;
    case ND_EXPR_STMT:
        normalize_expr(n, stmt->expr, ir_func);
        break;
    case ND_USING:
    case ND_MATCH_TYPE:
        if(stmt->body)
            normalize_stmt(n, stmt->body, ir_func);
        break;
    case ND_ASM:
        ir_stmt = init_ir_stmt(n->context, IR_STMT_ASM);
        ir_stmt->_asm.args = init_list_sized(stmt->args->size);
        for(size_t i = 0; i < stmt->args->size; i++)
            list_push(ir_stmt->_asm.args, normalize_expr(n, stmt->args->items[i], ir_func));
        break;
    default:
        throw_error(n->context, ERR_CODEGEN_WARN, stmt->tok, "normalization not implemented");
        break;
    }

    if(ir_stmt)
        list_push(ir_func->stmts, ir_stmt);
}

static void normalize_function(ASTObj_T* func, va_list args)
{
    GET_NORMALIZER(args);

    IRFunction_T* ir_func = init_ir_function(
        n->context,
        func->tok,
        func->id->callee /* TODO */,
        func->is_extern || func->is_extern_c,
        func->data_type->is_variadic,
        normalize_type(n, func->return_type)
    );
    
    ir_func->params = init_list_sized(func->args->size); 
    CONTEXT_ALLOC_REGISTER(n->context, ir_func->params);

    n->locals_info = (LocalsInfo_T){
        .local_id = 0,
    };

    for(size_t i = 0; i < func->args->size; i++)
    {
        ASTObj_T* arg = func->args->items[i];
        IRParameter_T* ir_param = init_ir_parameter(n->context, n->locals_info.local_id++, normalize_type(n, arg->data_type));
        list_push(ir_func->params, ir_param);
    }

    list_push(n->ir->functions, ir_func);

    if(ir_func->is_extern)
        return; 

    for(size_t i = 0; i < func->objs->size; i++)
    {
        ASTObj_T* local = func->objs->items[i];
        
        IRLocal_T* ir_local = init_ir_local(n->context, n->locals_info.local_id++, normalize_type(n, local->data_type));
        list_push(ir_func->locals, ir_local);
    }

    n->label_id = 0;
    normalize_stmt(n, func->body, ir_func); 

    if(ir_func->return_type->kind == IR_TYPE_VOID)
        list_push(ir_func->stmts, init_ir_stmt(n->context, IR_STMT_RETURN));
}

static void normalize_global(ASTObj_T* global, va_list args)
{
    GET_NORMALIZER(args);

    list_push(n->ir->globals, init_ir_global(
        n->context,
        global->tok,
        global->id->callee /* TODO */,
        global->is_extern || global->is_extern_c,
        global->is_constant,
        normalize_type(n, global->data_type),
        global->value ? normalize_basic_expr(n, global->value) : NULL
    ));
}

static void normalize_enum_member(ASTObj_T* member, va_list args)
{

}


