#include "validator.h"
#include "ast/ast_iterator.h"
#include "config.h"
#include "error/error.h"
#include "hashmap.h"
#include "lexer/token.h"
#include "list.h"
#include "optimizer/constexpr.h"
#include "ast/ast.h"
#include "parser/queue.h"
#include "timer/timer.h"
#include "util.h"

#include <asm-generic/errno-base.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define GET_VALIDATOR(va) Validator_T* v = va_arg(va, Validator_T*)

typedef struct IDENT_RESOLVE_RESULT_STRUCT {
    bool found;
    union {
        ASTObj_T* obj;
        ASTIdentifier_T* error_on;
    };
} IdentResolveResult_T;

#define IDENT_FOUND(obj_) \
    ((IdentResolveResult_T){.found = true, .obj = (obj_)})
#define IDENT_MISSING(error_on_) \
    ((IdentResolveResult_T){.found = false, .error_on = (error_on_)})

static void begin_scope(Validator_T* v,  List_T* objs);
static void end_scope(Validator_T* v);
static void scope_register_obj(Validator_T* v, ASTObj_T* obj);
static IdentResolveResult_T scope_resolve_ident(Validator_T* v, Scope_T* scope, ASTIdentifier_T* ident);

static void validate_idents(Validator_T* v);

static void resolve_types(Validator_T* v, ResolveQueue_T* queue);

// validator struct functions
static void init_validator(Validator_T* v, Context_T* context, ASTProg_T* ast)
{
    memset(v, 0, sizeof(struct VALIDATOR_STRUCT));
    v->context = context;
    v->ast = ast;
    v->obj_stack = init_list();
    init_constexpr_resolver(&v->constexpr_resolver, context, ast);
}

static void free_validator(Validator_T* v)
{
    free_constexpr_resolver(&v->constexpr_resolver);
    free_list(v->obj_stack);
}


void validator_push_obj(Validator_T* v, ASTObj_T* obj)
{
    list_push(v->obj_stack, v->current_obj);
    v->current_obj = obj;
}

void validator_pop_obj(Validator_T* v)
{
    if(v->obj_stack->size == 0)
    {
        v->current_obj = NULL;
        return;
    }
    v->current_obj = list_pop(v->obj_stack);
}

#define RETURN_IF_ERRORED(context) \
    if((context)->emitted_errors) goto finish;

i32 validator_pass(Context_T* context, ASTProg_T* ast)
{
    timer_start(context, "code validation");

    // initialize the validator
    Validator_T v;
    init_validator(&v, context, ast);
    context->current_obj = &v.current_obj;

    begin_scope(&v, ast->objs);
    v.global_scope = v.current_scope;

    validate_idents(&v); 
    RETURN_IF_ERRORED(context);

    ResolveQueue_T queue;
    resolve_queue_init(&queue);
    build_resolve_queue(&v, &queue);
    RETURN_IF_ERRORED(context);

    resolve_types(&v, &queue);

    ResolveQueueNode_T* node = queue.head;
    char* buf = malloc(BUFSIZ * sizeof(char));
    size_t i = 1;
    printf("queued objs:\n");
    while(node) {
        *buf = '\0';
        ast_id_to_str(buf, node->obj->id, BUFSIZ);
        printf("[%02zu] %s %d\n", i++, buf, node->method);
        node = node->next;
    }

    resolve_queue_free(&queue);

    end_scope(&v);
    assert(v.scope_depth == 0);
/*
    // end the validator
    v.global_scope = NULL;
    v.current_function = NULL;

    // check for the main function
    check_exit_fns(&v);
    if(!v.main_function_found && context->flags.require_entrypoint)
    {
        LOG_ERROR(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " missing entrypoint; no `main` function declared.\n");
        context->emitted_errors++;
    }*/


    printf("validator not implemented.\n");
 
    exit(1);
    
finish:
    free_validator(&v);
    timer_stop(context);

    return context->emitted_errors;
}

static ASTObj_T* check_is_deprecated(Validator_T* v, ASTObj_T* obj, Token_T* location)
{
    if(obj->deprecated)
        throw_error(
            v->context,
            ERR_DEPRECATED,
            location,
            "%s `%s` is deprecated",
            obj_kind_to_str(obj->kind),
            obj->id->callee
        );
    return obj;
}

static void report_missing_ident(Context_T* context, IdentResolveResult_T* result, const char* what)
{
    if(result->found)
        return;
    
    throw_error(context, ERR_UNDEFINED_UNCR, result->error_on->tok, "%s `%s`", what, result->error_on->callee);
}

static ASTType_T* expand_typedef(Validator_T* v, ASTType_T* type)
{
    if(!type || type->kind != TY_UNDEF)
        return type;
    if(type->base)
        return type->base;

    IdentResolveResult_T typedef_result = scope_resolve_ident(v, v->current_scope, type->id);
    report_missing_ident(v->context, &typedef_result, "undefined data type");
    ASTObj_T* type_def = typedef_result.obj;
    if(type_def->kind != OBJ_TYPEDEF)
        throw_error(v->context, ERR_TYPE_ERROR_UNCR, type->tok, "identifier `%s` references object of kind %s, expect type.", type->id->callee, obj_kind_to_str(type_def->kind));
    return expand_typedef(v, type_def->data_type);
}

static void begin_scope(Validator_T* v, List_T* objs)
{
    Scope_T* scope = malloc(sizeof(Scope_T));
    scope->objs = hashmap_init();
    scope->outer = v->current_scope;
    v->current_scope = scope;
    v->scope_depth++;

    if(objs == NULL)
        return;

    for(size_t i = 0; i < objs->size; i++)
        scope_register_obj(v, objs->items[i]);
}

static void end_scope(Validator_T* v)
{
    assert(v->scope_depth > 0);
    
    Scope_T* scope = v->current_scope;
    v->current_scope = scope->outer;
    v->scope_depth--;
    
    hashmap_free(scope->objs);
    free(scope);
}

static void scope_register_obj(Validator_T* v, ASTObj_T* obj)
{
    assert(v->scope_depth > 0);
    if(hashmap_put(v->current_scope->objs, obj->id->callee, obj) == EEXIST)
    {
        ASTObj_T* existing = hashmap_get(v->current_scope->objs, obj->id->callee);
        assert(existing != NULL);

        throw_error(
            v->context,
            ERR_REDEFINITION_UNCR,
            obj->id->tok,
            "redefinition of %s `%s`.\nfirst defined in " COLOR_BOLD_WHITE "%s" COLOR_RESET " at line " COLOR_BOLD_WHITE "%u" COLOR_RESET " as %s.",
            obj_kind_to_str(obj->kind),
            obj->id->callee,
            EITHER(existing->tok->source->short_path, existing->tok->source->path),
            existing->tok->line + 1,
            obj_kind_to_str(existing->kind)
        );
    }
}

static void scope_register_node(Validator_T* v, ASTNode_T* node)
{
    assert(v->scope_depth > 0);
    if(hashmap_put(v->current_scope->objs, node->id->callee, node) == EEXIST)
    {
        ASTNode_T* existing = hashmap_get(v->current_scope->objs, node->id->callee);
        assert(existing != NULL);

        throw_error(
            v->context,
            ERR_REDEFINITION_UNCR,
            node->id->tok,
            "redefinition of member `%s`.\nfirst defined in " COLOR_BOLD_WHITE "%s" COLOR_RESET " at line " COLOR_BOLD_WHITE "%u" COLOR_RESET ".",
            node->id->callee,
            EITHER(existing->tok->source->short_path, existing->tok->source->path),
            existing->tok->line + 1
        );
    }
}

static ASTObj_T* scope_contains(Scope_T* scope, const char* id)
{
    return hashmap_get(scope->objs, id);
}

static IdentResolveResult_T scope_resolve_ident(Validator_T* v, Scope_T* scope, ASTIdentifier_T* ident)
{
    if(ident->global_scope)
        scope = v->global_scope;
    if(!scope || !ident)
        return IDENT_MISSING(ident);

    if(ident->outer)
    {
        IdentResolveResult_T outer_result = scope_resolve_ident(v, scope, ident->outer);
        if(!outer_result.found)
            return IDENT_MISSING(EITHER(ident->outer, ident));
        
        switch(outer_result.obj->kind)
        {
        case OBJ_TYPEDEF:
            {
                ASTType_T* expanded = expand_typedef(v, outer_result.obj->data_type);
                if(expanded->kind == TY_ENUM)
                    for(size_t i = 0; i < expanded->members->size; i++)
                    {
                        ASTObj_T* member = expanded->members->items[i];
                        if(strcmp(member->id->callee, ident->callee) == 0)
                            return IDENT_FOUND(check_is_deprecated(v, member, ident->tok));
                    }
                throw_error(v->context, ERR_UNDEFINED_UNCR, ident->outer->tok, "type `%s` has no member called `%s`", outer_result.obj->id->callee, ident->callee);
            } break;
        case OBJ_NAMESPACE:
            for(size_t i = 0; i < outer_result.obj->objs->size; i++)
            {
                ASTObj_T* obj = outer_result.obj->objs->items[i];
                if(strcmp(obj->id->callee, ident->callee) == 0)
                    return IDENT_FOUND(check_is_deprecated(v, obj, ident->tok));
            }
            break;
        default:
            break;
        }

        return IDENT_MISSING(ident);
    }
    else
    {
        ASTObj_T* found = scope_contains(scope, ident->callee);
        if(found)
            return IDENT_FOUND(check_is_deprecated(v, found, ident->tok));
        return scope_resolve_ident(v, scope->outer, ident);
    }
}

static void enter_namespace_scope(ASTObj_T* namespace, va_list args)
{
    GET_VALIDATOR(args);
    validator_push_obj(v, namespace);
    begin_scope(v, namespace->objs);
}

static void leave_namespace_scope(ASTObj_T* namespace, va_list args)
{
    GET_VALIDATOR(args);
    validator_pop_obj(v);
    end_scope(v);
}

static void enter_function_scope(ASTObj_T* function, va_list args)
{
    GET_VALIDATOR(args);

    validator_push_obj(v, function);
    begin_scope(v, function->args);
    if(function->data_type->is_variadic && !function->is_extern)
        scope_register_obj(v, function->va_area);
}

static void leave_function_scope(ASTObj_T* function, va_list args)
{
    GET_VALIDATOR(args);
    validator_pop_obj(v);
    end_scope(v);
}

static void enter_type_scope(ASTType_T* type, va_list args)
{
    GET_VALIDATOR(args);
    switch(type->kind)
    {
    case TY_ENUM:
        begin_scope(v, type->members);
        break;
    case TY_STRUCT:
        begin_scope(v, NULL);
        for(size_t i = 0; i < type->members->size; i++)
            scope_register_node(v, type->members->items[i]);
        break;
    default:
        break;
    }
}

static void leave_type_scope(ASTType_T* type, va_list args)
{
    GET_VALIDATOR(args);
    switch(type->kind)
    {
    case TY_ENUM:
    case TY_STRUCT:
        end_scope(v);
        break;
    default:
        break;
    }
}

static void enter_block_scope(ASTNode_T* block, va_list args)
{
    GET_VALIDATOR(args);
    begin_scope(v, NULL);
}

static void leave_block_scope(ASTNode_T* block, va_list args)
{
    GET_VALIDATOR(args);
    end_scope(v);
}

static void enter_lambda_scope(ASTNode_T* lambda, va_list args)
{
    GET_VALIDATOR(args);
    begin_scope(v, lambda->args);
}

static void leave_lambda_scope(ASTNode_T* lambda, va_list args)
{
    GET_VALIDATOR(args);
    end_scope(v);
}

static void enter_using_scope(ASTNode_T* using, va_list args)
{
    GET_VALIDATOR(args);
    if(using->body)
        begin_scope(v, NULL);

    for(size_t i = 0; i < using->ids->size; i++)
    {
        ASTIdentifier_T* id = using->ids->items[i];
        IdentResolveResult_T result = scope_resolve_ident(v, v->current_scope, id);
        if(!result.found)
        {
            report_missing_ident(v->context, &result, "using undefined namespace");
            return;
        }

        ASTObj_T* namespace = result.obj;
        if(namespace->kind != OBJ_NAMESPACE)
        {
            throw_error(v->context, ERR_UNDEFINED_UNCR, id->tok, "`%s` is a %s, can only import symbols from namespaces using `using`", id->callee, obj_kind_to_str(namespace->kind));
            return;
        }

        for(size_t i = 0; i < namespace->objs->size; i++)
        {
            ASTObj_T* obj = namespace->objs->items[i];
            if(hashmap_put(v->current_scope->objs, obj->id->callee, obj) == EEXIST)
                throw_error(
                    v->context,
                    ERR_REDEFINITION_UNCR,
                    id->tok,
                    "namespace `%s` is trying to implement a %s `%s`,\nwhich is already defined in this scope",
                    namespace->id->callee,
                    obj_kind_to_str(obj->kind),
                    obj->id->callee
                );
        }
    }
}

static void leave_using_scope(ASTNode_T* using, va_list args)
{
    GET_VALIDATOR(args);
    if(using->body)
        end_scope(v);
}

static void register_local_var(ASTObj_T* local, va_list args)
{
    GET_VALIDATOR(args);
    if(v->current_obj->kind == OBJ_FUNCTION && v->current_obj->va_area == local)
        return;
    scope_register_obj(v, local);
}

static void push_global_var(ASTObj_T* global, va_list args)
{
    GET_VALIDATOR(args);
    validator_push_obj(v, global);
}

static void pop_global_var(ASTObj_T* global, va_list args)
{
    GET_VALIDATOR(args);
    validator_pop_obj(v);
}

static void validate_ident_expr(ASTNode_T* node, va_list args)
{
    GET_VALIDATOR(args);
    IdentResolveResult_T result = scope_resolve_ident(v, v->current_scope, node->id);
    if(!result.found)
    {
        report_missing_ident(v->context, &result, "referring to undefined identifier");
        return;
    }

    node->referenced_obj = result.obj;
    if(node->referenced_obj->private && node->id->outer)
        throw_error(v->context, ERR_CALL_ERROR_UNCR, node->id->tok, "referring to private %s `%s`", obj_kind_to_str(node->referenced_obj->kind), node->id->callee);
    
    if(node->referenced_obj->id->outer)
        node->id->outer = node->referenced_obj->id->outer;

    if(v->current_obj && v->current_obj->constexpr && !node->referenced_obj->constexpr)
    {
        char buf[BUFSIZ] = {'\0'};
        throw_error(v->context, ERR_CONSTEXPR, node->tok, "%s `%s` is not marked as `[constexpr]`", obj_kind_to_str(node->referenced_obj->kind), ast_id_to_str(buf, node->referenced_obj->id, LEN(buf)));
    }
}

static void validate_undef_type_ident(ASTType_T* type, va_list args)
{
    GET_VALIDATOR(args);
    IdentResolveResult_T result = scope_resolve_ident(v, v->current_scope, type->id);
    if(!result.found)
    {
        report_missing_ident(v->context, &result, "could not find data type named");
        return;
    }

    ASTObj_T* found = result.obj;
    if(found->kind != OBJ_TYPEDEF)
    {
        throw_error(v->context, ERR_TYPE_ERROR_UNCR, type->tok, "%s `%s` is not a type", obj_kind_to_str(found->kind), type->id->callee);
        return;
    }

    type->referenced_obj = found;
    type->id->outer = found->id->outer;
    type->base = found->data_type;
}

static void validate_idents(Validator_T* v)
{
    static const ASTIteratorList_T iter = {
        .obj_start_fns = {
            [OBJ_NAMESPACE] = enter_namespace_scope,
            [OBJ_FUNCTION] = enter_function_scope,
            [OBJ_GLOBAL] = push_global_var,
            [OBJ_LOCAL] = register_local_var,
        },
        .obj_end_fns = {
            [OBJ_NAMESPACE] = leave_namespace_scope,
            [OBJ_FUNCTION] = leave_function_scope,
            [OBJ_GLOBAL] = pop_global_var,
        },
        .node_start_fns = {
            [ND_ID] = validate_ident_expr,
            [ND_BLOCK] = enter_block_scope,
            [ND_LAMBDA] = enter_lambda_scope,
            [ND_USING] = enter_using_scope,
        },
        .node_end_fns = {
            [ND_BLOCK] = leave_block_scope,
            [ND_LAMBDA] = leave_lambda_scope,
            [ND_USING] = leave_using_scope,
        },
        .type_fns = {
            [TY_UNDEF] = validate_undef_type_ident
        },
        .type_begin = enter_type_scope,
        .type_end = leave_type_scope,
    };

    ast_iterate(&iter, v->ast, v);
}

static void resolve_types(Validator_T* v, ResolveQueue_T* queue)
{

}
