#include "optimizer.h"

#include "../io/log.h"
#include "../ast/types.h"

#include <stdio.h>
#include <string.h>

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)

#define TYPENAME_REDEF(obj, found)                                                                                          \
        throw_error(ERR_REDEFINITION, obj->tok, "redefinition of typename \"%s\", already defined as: at: [%s%s%ld:%ld]",   \
            obj->callee,                                                                                                    \
            obj_kind_to_str(found->kind),                                                                                   \
            strcmp(found->tok->source->path, obj->tok->source->path) == 0 ? "" : obj->tok->source->path,                    \
            strcmp(found->tok->source->path, obj->tok->source->path) == 0 ? "" : ":",                                       \
            obj->tok->line + 1,                                                                                             \
            obj->tok->pos + 1                                                                                               \
)

Optimizer_T* init_optimizer(void)
{
    Optimizer_T* o = malloc(sizeof(struct OPTIMIZER_STRUCT));
    o->num_errors_found = 0;

    o->scope = malloc(sizeof(struct SCOPE_STRUCT)); 
    o->scope->objs = init_list(sizeof(struct AST_OBJ_STRUCT*)),
    o->scope->depth = 0;
    
    o->cur_fn = NULL;

    return o;
}

void free_optimizer(Optimizer_T* o)
{
    free_list(o->scope->objs);
    free(o->scope);
    free(o);
}

static inline void begin_scope(Scope_T* scope) { scope->depth++; }
static inline void end_scope(Scope_T* scope) { scope->depth--; }
static inline void pop_obj(Scope_T* scope) { scope->objs->size--; }
static inline size_t scope_size(Scope_T* scope) { return scope->objs->size; }
static void optimize_expr(Optimizer_T* o, ASTNode_T* expr);

static ASTObj_T* find_obj(Scope_T* scope, char callee[__CSP_MAX_TOKEN_SIZE]);

static inline void push_obj(Scope_T* scope, ASTObj_T* obj) 
{   
    ASTObj_T* found = find_obj(scope, obj->callee);
    if(found)
        TYPENAME_REDEF(obj, found);
    else
        list_push(scope->objs, obj); 
}

static inline ASTObj_T* want_fn(ASTObj_T* obj, Token_T* call_tok) 
{ 
    if(!obj)
        throw_error(ERR_UNDEFINED, call_tok, "undefined function `%s`", call_tok->value);
    else if(obj->kind == OBJ_FUNCTION) 
        return obj; 
    else 
        throw_error(ERR_TYPE_ERROR, call_tok, "\"%s\" is expected to be a function, got: %s", obj->callee, obj_kind_to_str(obj->kind)); 
    return NULL;
}

static inline ASTObj_T* want_var(ASTObj_T* obj, Token_T* call_tok) 
{
    if(!obj)
        throw_error(ERR_UNDEFINED, call_tok, "undefined variable `%s`", call_tok->value);
    else if(obj->kind == OBJ_FN_ARG || obj->kind == OBJ_LOCAL || obj->kind == OBJ_GLOBAL)
        return obj;
    else
        throw_error(ERR_TYPE_ERROR, call_tok, "\"%s\" is expected to be a variable, got: %s", obj->callee, obj_kind_to_str(obj->kind)); 
    return NULL;
}

static inline ASTObj_T* want_typedef(ASTObj_T* obj, Token_T* call_tok) 
{
    if(!obj)
        throw_error(ERR_UNDEFINED, call_tok, "undefined type `%s`", call_tok->value);
    else if(obj->kind == OBJ_TYPEDEF)
        return obj;
    else
        throw_error(ERR_TYPE_ERROR, call_tok, "\"%s\" is expected to be a type definition, got: %s", obj->callee, obj_kind_to_str(obj->kind)); 
    return NULL;
}

static void register_fn(Optimizer_T* o, ASTObj_T* fn);
static void register_var(Optimizer_T* o, ASTObj_T* gl);
static void register_tdef(Optimizer_T* o, ASTObj_T* ty);

static void check_main_fn(Optimizer_T* o);
static void optimize_stmt(Optimizer_T* o, ASTNode_T* stmt);
static void optimize_local(Optimizer_T* o, ASTObj_T* local);

void optimize(ASTProg_T* ast)
{
    Optimizer_T* o = init_optimizer();

    for(size_t i = 0; i < ast->objs->size; i++)
    {
        ASTObj_T* obj = ast->objs->items[i];
        switch(obj->kind)
        {
            case OBJ_FUNCTION:
                register_fn(o, obj);
                break;
            case OBJ_GLOBAL:
                register_var(o, obj);
                break;
            case OBJ_TYPEDEF:
                register_tdef(o, obj);
                break;
            default:
                break;
        }
    }
    
    check_main_fn(o);

    for(size_t i = 0; i < ast->objs->size; i++)
    {
        ASTObj_T* fn = ast->objs->items[i];
        if(fn->kind != OBJ_FUNCTION || fn->is_extern)
            continue;
        
        o->cur_fn = fn;
        
        if(fn->body->kind != ND_BLOCK)
        {
            throw_error(ERR_MISC, fn->body->tok, "unexpected statement of type `%d`, expect block", fn->body->kind);
            o->num_errors_found++;
            continue;
        }
        bool has_return = false;

        for(size_t i = 0; i < fn->body->locals->size; i++)
            optimize_local(o, fn->body->locals->items[i]);

        for(size_t i = 0; i < fn->body->stmts->size; i++)
        {
            ASTNode_T* stmt = fn->body->stmts->items[i];
            optimize_stmt(o, stmt);
            if(stmt->kind == ND_RETURN)
                has_return = true;
        }

        if(!has_return && fn->return_type->kind != TY_VOID)
        {
            throw_error(ERR_SYNTAX_WARNING, fn->tok, "function \"%s\" with non-void return type does not return a value", fn->callee);
            o->num_errors_found++;
        }
    }

    if(o->num_errors_found > 0)
    {
        LOG_ERROR_F("Encountered %d error%s during compilation\n", o->num_errors_found, o->num_errors_found == 1 ? "" : "s");
        free_optimizer(o);
        exit(1);
    }

    free_optimizer(o);
}

static ASTObj_T* find_obj(Scope_T* scope, char callee[__CSP_MAX_TOKEN_SIZE])
{
    for(size_t i = 0; i < scope_size(scope); i++)
        if(strcmp(((ASTObj_T*)scope->objs->items[i])->callee, callee) == 0)
            return scope->objs->items[i];
    return NULL;
}

static void register_fn(Optimizer_T* o, ASTObj_T* fn)
{
    ASTObj_T* found = find_obj(o->scope, fn->callee);
    if(found) 
    {
        throw_error(ERR_REDEFINITION, fn->tok, "redefinition of function \"%s\", first defined in" COLOR_BOLD_WHITE " %s:[%ld:%ld]" COLOR_RESET, fn->callee, found->tok->source->short_path, found->tok->line, found->tok->pos);
        o->num_errors_found++;
    }

    push_obj(o->scope, fn);
}

static void register_var(Optimizer_T* o, ASTObj_T* var)
{
    ASTObj_T* found = find_obj(o->scope, var->callee);
    if(found)
    {
        throw_error(ERR_REDEFINITION, var->tok, "redefinition of variable \"%s\", first defined in" COLOR_BOLD_WHITE " %s:[%ld:%ld]" COLOR_RESET, var->callee, found->tok->source->short_path, found->tok->line, found->tok->pos);
        o->num_errors_found++;
    }

    push_obj(o->scope, var);
}

static void register_tdef(Optimizer_T* o, ASTObj_T* ty)
{
    if(get_primitive_type(ty->callee))
    {
        throw_error(ERR_REDEFINITION, ty->tok, "redefinition of type \"%s\", cannot overwrite a primitive", ty->callee);
        o->num_errors_found++;
    }

    ASTObj_T* found = find_obj(o->scope, ty->callee);
    if(found)
    {
        throw_error(ERR_REDEFINITION, ty->tok, "redefinition of type \"%s\", first defined in" COLOR_BOLD_WHITE " %s:[%ld:%ld]" COLOR_RESET, ty->callee, "TODO", found->tok->line, found->tok->pos);
        o->num_errors_found++;
    }
    
    push_obj(o->scope, ty);
}

static void check_main_fn(Optimizer_T* o)
{
    char main_name[__CSP_MAX_TOKEN_SIZE] = "main";
    ASTObj_T* main_fn = find_obj(o->scope, main_name);
    if(!main_fn)
    {
        LOG_ERROR("Missing entry point: could not find function \"main\".\n");
        o->num_errors_found++;
        return;
    }
    main_fn = want_fn(main_fn, main_fn->tok);

    if(main_fn->args->size != 0 && main_fn->args->size != 2)
    {
        throw_error(ERR_MISC, main_fn->tok, "arong number of arguments for function \"main\", expect [0, 2]");
        o->num_errors_found++;
        return;
    }

    if(main_fn->return_type->kind != TY_I32)
    {
        throw_error(ERR_MISC, main_fn->return_type->tok, "function \"main\" has to return type `i32`");
        o->num_errors_found++;
    }

    if(main_fn->args->size == 2) {
        ASTObj_T* arg1 = (ASTObj_T*)main_fn->args->items[0];
        if(arg1->data_type->kind != TY_I32)
        {
            throw_error(ERR_MISC, arg1->data_type->tok, "argument 1 of function \"main\" has to be of type `i32`");
            o->num_errors_found++;
        }

        ASTObj_T* arg2 = (ASTObj_T*)main_fn->args->items[1];
        if(arg2->data_type->kind != TY_PTR || arg2->data_type->base->kind != TY_PTR || arg2->data_type->base->base->kind != TY_CHAR)
        {
            throw_error(ERR_MISC, arg2->data_type->tok, "argument 2 of function \"main\" has to be of type `**char`");
            o->num_errors_found++;
        }
    }
}

static void optimize_local(Optimizer_T* o, ASTObj_T* local)
{
    push_obj(o->scope, local);
}

static void optimize_stmt(Optimizer_T* o, ASTNode_T* stmt)
{
    switch(stmt->kind)
    {
        case ND_BLOCK:
            for(size_t i = 0; i < stmt->locals->size; i++)
                optimize_local(o, stmt->locals->items[i]);
            for(size_t i = 0; i < stmt->stmts->size; i++)
                optimize_stmt(o, stmt->stmts->items[i]);
            break;
        case ND_EXPR_STMT:
            break;
        case ND_RETURN:
            if(stmt->return_val)
            {
                optimize_expr(o, stmt->return_val);
                if(!check_type_compatibility(stmt->return_val->data_type, o->cur_fn->return_type))
                    throw_error(ERR_ILLEGAL_TYPE_CAST, stmt->return_val->tok, "return statement of type %d is not compatible with function return type %d", stmt->return_val->kind, o->cur_fn->return_type);
            }
            break;
        case ND_IF:
            break;
        case ND_MATCH:
            break;
        case ND_LOOP:
            break;  
        case ND_NOOP:
            break;
        default:
            break;
    }
}

static void optimize_expr(Optimizer_T* o, ASTNode_T* expr)
{
    switch(expr->kind)
    {
        case ND_ID:
            {
                ASTObj_T* obj = want_var(find_obj(o->scope, expr->callee), expr->tok);
                expr->data_type = obj->data_type;
            }
        // TODO: evaluate all expressions
        default:
            break;
    }
}