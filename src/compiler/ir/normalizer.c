#include "normalizer.h"
#include "ast/ast.h"
#include "ast/ast_iterator.h"
#include "ast/types.h"
#include "codegen/codegen_utils.h"
#include "config.h"
#include "context.h"
#include "error/error.h"
#include "ir/debug.h"
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

static void free_type_pairs(List_T** pairs) {
    for(size_t i = 0; i < (*pairs)->size; i++)
        free((*pairs)->items[i]);
    free_list(*pairs);
    *pairs = NULL;
}

static IRType_T* get_ir_type(Normalizer_T* n, ASTType_T* ast_type)
{
    assert(ast_type != NULL);

    size_t i;
    for(i = 0; i < n->type_pairs->size; i++)
    {
        TypePair_T* pair = n->type_pairs->items[i];
        if(pair->ast_type == ast_type)
            return pair->ir_type;
    }
    
    for(i = 0; i < n->type_pairs->size; i++)
    {
        TypePair_T* pair = n->type_pairs->items[i];
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
        .type_pairs = init_list()
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

    if(context->flags.verbose)
    {
        LOG_INFO(COLOR_BOLD_MAGENTA ">> IR Normalization:\n" COLOR_RESET);
        dbg_print_ir(ir, IR_PRINT_FUNC);
    }

    free_type_pairs(&normalizer.type_pairs);

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
    list_push(n->type_pairs, new_type_pair(type, ir_type));
    list_push(n->ir->types, ir_type);

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
            list_push(ir_type->fields, field);
        }
        break;
    default:
        break;
    }

    return ir_type;
}

static IRRegister_T next_register(Normalizer_T* n, ASTObj_T* backing) {
    list_push(n->func_context.register_backings, backing);
    return (IRRegister_T){
        n->func_context.register_id++
    };
}

static IRRegister_T normalize_local(Normalizer_T* n, ASTObj_T* local, IRFunction_T* ir_func) {
    assert(local->kind == OBJ_LOCAL);

    IRStmt_T* decl = init_ir_stmt(n->context, IR_STMT_DECL);

    decl->decl.reg = next_register(n, local);
    init_ir_lvalue(&decl->decl.value, IR_LVALUE_ALLOCA, normalize_type(n, local->data_type));

    list_push(ir_func->stmts, decl);

    return decl->decl.reg;
}

static IRLiteral_T* void_literal(Normalizer_T* n, IRLiteral_T* lit) {
    init_ir_literal(lit, IR_LITERAL_VOID, normalize_type(n, (ASTType_T*) primitives[TY_VOID]));
    return lit;
}

static IRLiteral_T* normalize_literal(Normalizer_T* n, IRLiteral_T* ir_lit, ASTNode_T* expr)
{
    init_ir_literal(ir_lit, -1, normalize_type(n, expr->data_type));
    switch(expr->kind)
    {
    case ND_INT:
        ir_lit->kind = IR_LITERAL_I32;
        ir_lit->i32_lit = expr->int_val;
        break;
    
    case ND_LONG:
        ir_lit->kind = IR_LITERAL_I64;
        ir_lit->i64_lit = expr->long_val;
        break;

    case ND_ULONG:
        ir_lit->kind = IR_LITERAL_U64;
        ir_lit->u64_lit = expr->ulong_val;
        break;

    case ND_FLOAT:
        ir_lit->kind = IR_LITERAL_F32;
        ir_lit->f32_lit = expr->float_val;
        break;

    case ND_DOUBLE:
        ir_lit->kind = IR_LITERAL_F64;
        ir_lit->f64_lit = expr->double_val;
        break;

    case ND_CHAR:
        ir_lit->kind = IR_LITERAL_I8;
        ir_lit->i8_lit = (i8) expr->int_val;
        break;

    case ND_BOOL:
        ir_lit->kind = IR_LITERAL_U8;
        ir_lit->u8_lit = (u8) expr->bool_val;
        break;

    default:
        throw_error(n->context, ERR_INTERNAL, expr->tok, "unexpected expr in normalization process");
    }

    assert(((i32) ir_lit->kind) != -1);
    return ir_lit;
}

static u64 make_label(Normalizer_T* n, IRFunction_T* ir_func)
{
    IRStmt_T* label = init_ir_stmt(n->context, IR_STMT_LABEL);
    label->label.id = n->label_id++;

    list_push(ir_func->stmts, label);
    return label->label.id;
}

static void make_goto(Normalizer_T* n, IRFunction_T* ir_func, u64 label_id, IRLiteral_T cond, bool negate)
{
    IRStmt_T* _goto = init_ir_stmt(n->context, cond.kind == IR_LITERAL_VOID ? IR_STMT_GOTO : IR_STMT_GOTO_IF);

    if(cond.type != IR_LITERAL_VOID)
    {
        _goto->goto_if.label_id = label_id;
        _goto->goto_if.condition = cond;
        _goto->goto_if.negated = negate;
    }
    else
        _goto->_goto.label_id = label_id;

    list_push(ir_func->stmts, _goto);
}

static IRRegister_T get_register(Normalizer_T* n, ASTObj_T* backing) {
    for(size_t i = 0; i < n->func_context.register_backings->size; i++) {
        ASTObj_T* cur = n->func_context.register_backings->items[i];
        if(cur == backing)
            return (IRRegister_T){(u32) i};
    }

    assert(false);
    return (IRRegister_T){UINT32_MAX};
}

static IRLiteral_T* normalize_ident(Normalizer_T* n, IRLiteral_T* lit, ASTNode_T* ident, IRFunction_T* ir_func)
{
    assert(ident->referenced_obj);

    switch(ident->referenced_obj->kind) {
        case OBJ_LOCAL:
        case OBJ_FN_ARG:
            lit->kind = IR_LITERAL_DEREF;
            lit->type = normalize_type(n, ident->data_type);
            lit->deref.reg = get_register(n, ident->referenced_obj);
            lit->deref.offset = 0;
            break;
        default:
            throw_error(n->context, ERR_INTERNAL, ident->tok, "cannot generate IR for ident referencing object of type `%d` yet.", ident->referenced_obj->kind);
    }

    return lit;
}

static IRLiteral_T* normalize_expr(Normalizer_T* n, IRLiteral_T* lit, ASTNode_T* expr, IRFunction_T* ir_func);

static IRLiteral_T* normalize_assignment(Normalizer_T* n, IRLiteral_T* lit, ASTNode_T* expr, IRFunction_T* ir_func)
{
    switch(expr->left->kind) {
        case ND_ID:
            normalize_ident(n, lit, expr->left, ir_func);
            break;
        default:
            throw_error(n->context, ERR_INTERNAL, expr->tok, "could not generate IR for assignment lvalue of kind %d.", expr->left->kind);
            unreachable();
    }

    assert(lit->kind == IR_LITERAL_DEREF);

    IRStmt_T* ir_stmt = init_ir_stmt(n->context, IR_STMT_STORE_DEREF);
    ir_stmt->store_deref.location = lit->deref.reg;
    ir_stmt->store_deref.offset = lit->deref.offset;

    normalize_expr(n, &ir_stmt->store_deref.value, expr->right, ir_func);

    list_push(ir_func->stmts, ir_stmt);

    return lit;
}

static IRLiteral_T* normalize_expr(Normalizer_T* n, IRLiteral_T* lit, ASTNode_T* expr, IRFunction_T* ir_func)
{
    switch(expr->kind)
    {
        case ND_ASSIGN:
            return normalize_assignment(n, lit, expr, ir_func);
        case ND_ID:
            return normalize_ident(n, lit, expr, ir_func);
        default:
            return normalize_literal(n, lit, expr);
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
            normalize_expr(n, &ir_stmt->_return.lit, stmt->return_val, ir_func);
        else
            void_literal(n, &ir_stmt->_return.lit);
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

            IRLiteral_T void_lit;
            void_literal(n, &void_lit);
            make_goto(n, ir_func, continue_label, void_lit, false);

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

            make_goto(n, ir_func, break_label, 
                *normalize_expr(n, &(IRLiteral_T){}, stmt->condition, ir_func), 
            true);
            
            normalize_stmt(n, stmt->body, ir_func);
            
            IRLiteral_T void_lit;
            void_literal(n, &void_lit);
            make_goto(n, ir_func, continue_label, void_lit, false);
            
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

            IRLiteral_T cond_lit;
            normalize_expr(n, &cond_lit, stmt->condition, ir_func);
            make_goto(n, ir_func, begin_label, cond_lit, false);
            
            ir_stmt = init_ir_stmt(n->context, IR_STMT_LABEL);
            ir_stmt->label.id = break_label;

            push_loop_context(n, outer);
        } break;
    case ND_IF:
        {
            u64 else_label = n->label_id++;
            u64 end_label = stmt->else_branch ? n->label_id++ : else_label;
            
            make_goto(n, ir_func, else_label, 
                *normalize_expr(n, &(IRLiteral_T){}, stmt->condition, ir_func),
            true);
            normalize_stmt(n, stmt->if_branch, ir_func);

            if(stmt->else_branch)
            {
                make_goto(n, ir_func, end_label, *void_literal(n, &(IRLiteral_T){}), false);

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

            make_goto(n, ir_func, skip_label, *normalize_expr(n, &(IRLiteral_T){}, stmt->condition, ir_func), false);
            normalize_stmt(n, stmt->body, ir_func);

            ir_stmt = init_ir_stmt(n->context, IR_STMT_LABEL);
            ir_stmt->label.id = skip_label;
        } break;
    // ND_WITH
    // ND_FOR
    // ND_FOR_RANGE
/*    case ND_CASE:
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
        } break;*/
    case ND_EXPR_STMT: {
        IRLiteral_T result;
        normalize_expr(n, &result, stmt->expr, ir_func);
    } break;
    case ND_USING:
    case ND_MATCH_TYPE:
        if(stmt->body)
            normalize_stmt(n, stmt->body, ir_func);
        break;
    default:
        throw_error(n->context, ERR_CODEGEN, stmt->tok, "normalization not implemented");
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

    n->func_context = (FunctionContext_T){
        .register_id = 0,
        .register_backings = init_list()
    };

    for(size_t i = 0; i < func->args->size; i++)
    {
        ASTObj_T* arg = func->args->items[i];
        IRParameter_T* ir_param = init_ir_parameter(n->context, arg->tok, normalize_type(n, arg->data_type));
        ir_param->reg = next_register(n, arg);
        list_push(ir_func->params, ir_param);
    }

    list_push(n->ir->functions, ir_func);

    if(ir_func->is_extern)
        return; 

    for(size_t i = 0; i < func->objs->size; i++)
    {
        ASTObj_T* local = func->objs->items[i];
        normalize_local(n, local, ir_func);
    }

    n->label_id = 0;
    normalize_stmt(n, func->body, ir_func); 

    if(ir_func->return_type->kind == IR_TYPE_VOID)
        list_push(ir_func->stmts, init_ir_stmt(n->context, IR_STMT_RETURN));

    free_list(n->func_context.register_backings);
}

static void normalize_global(ASTObj_T* global, va_list args)
{
    GET_NORMALIZER(args);

    IRLiteral_T lit;
    list_push(n->ir->globals, init_ir_global(
        n->context,
        global->tok,
        global->id->callee /* TODO */,
        global->is_extern || global->is_extern_c,
        global->is_constant,
        normalize_type(n, global->data_type),
        global->value ? *normalize_literal(n, &lit, global->value) : *void_literal(n, &lit)
    ));
}

static void normalize_enum_member(ASTObj_T* member, va_list args)
{

}


