#include "ir.h"
#include "config.h"
#include "context.h"
#include "list.h"

void ir_init(IR_T* ir, Context_T* context)
{
    ir->files = init_list();
    CONTEXT_ALLOC_REGISTER(context, ir->files);

    ir->globals = init_list(); 
    CONTEXT_ALLOC_REGISTER(context, ir->globals);

    ir->functions = init_list();
    CONTEXT_ALLOC_REGISTER(context, ir->functions);
}

IRFunction_T* init_ir_function(Context_T* context, Token_T* tok, const char* mangled_id, bool is_extern, bool variadic, IRType_T* return_type)
{
    IRFunction_T* function = malloc(sizeof(struct IR_FUNCTION_STRUCT));
    CONTEXT_ALLOC_REGISTER(context, (void*) function);

    function->tok = tok;
    function->mangled_id = mangled_id;
    function->is_extern = is_extern;
    function->variadic = variadic;
    function->return_type = return_type;
    
    if(!is_extern)
    {
        function->stmts = init_list();
        CONTEXT_ALLOC_REGISTER(context, function->stmts);

        function->locals = init_list();
        CONTEXT_ALLOC_REGISTER(context, function->locals);
    }

    return function;
}

IRLocal_T* init_ir_local(Context_T* context, u32 id, IRType_T* type)
{
    IRLocal_T* local = malloc(sizeof(struct IR_LOCAL_STRUCT));
    CONTEXT_ALLOC_REGISTER(context, (void*) local);

    local->type = type;
    local->id = id; 
    local->temporary = false;

    return local;
}

IRParameter_T* init_ir_parameter(Context_T* context, u32 id, IRType_T* type)
{
    IRParameter_T* param = malloc(sizeof(struct IR_PARAMETER_STRUCT));
    CONTEXT_ALLOC_REGISTER(context, (void*) param);

    param->id = id;
    param->type = type;

    return param;
}

IRGlobal_T* init_ir_global(Context_T* context, Token_T* tok, const char* mangled_id, bool is_extern, bool is_const, IRType_T* type, IRExpr_T* value)
{
    IRGlobal_T* global = malloc(sizeof(struct IR_GLOBAL_STRUCT));
    CONTEXT_ALLOC_REGISTER(context, (void*) global);

    global->tok = tok;
    global->mangled_id = mangled_id;
    global->is_extern = is_extern;
    global->is_const = is_const;
    global->type = type;
    global->value = value;

    return global;
}

IRExpr_T* init_ir_expr(Context_T* context, IRExprKind_T kind, IRType_T* type)
{
    IRExpr_T* expr = malloc(sizeof(struct IR_GLOBAL_STRUCT));
    CONTEXT_ALLOC_REGISTER(context, (void*) expr);

    expr->kind = kind;
    expr->type = type;

    return expr;
}

IRStmt_T* init_ir_stmt(Context_T* context, IRStmtKind_T kind)
{
    IRStmt_T* stmt = malloc(sizeof(struct IR_STMT_STRUCT));
    CONTEXT_ALLOC_REGISTER(context, (void*) stmt);

    stmt->kind = kind;
    return stmt;
}

