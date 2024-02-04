#include "validator.h"
#include "ast/ast_iterator.h"
#include "codegen/codegen_utils.h"
#include "config.h"
#include "context.h"
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
#include <math.h>
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

static i32 get_type_size(Validator_T* v, ASTType_T* type);
static i32 get_type_align(Validator_T* v, ASTType_T* type);

static void validate_semantics(Validator_T* v, ResolveQueue_T* queue);

// validator struct functions
static void init_validator(Validator_T* v, Context_T* context, ASTProg_T* ast)
{
    memset(v, 0, sizeof(struct VALIDATOR_STRUCT));
    v->context = context;
    v->ast = ast;
    v->obj_stack = init_list();
    v->exact_type_info_stack = init_list();
    init_constexpr_resolver(&v->constexpr_resolver, context, ast);
}

static void free_validator(Validator_T* v)
{
    free_constexpr_resolver(&v->constexpr_resolver);
    free_list(v->obj_stack);
    free_list(v->exact_type_info_stack);
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

void validator_push_type_info(Validator_T* v, bool need_exact)
{
    list_push(v->exact_type_info_stack, (void*) v->need_exact_type_infos);
    v->need_exact_type_infos = need_exact;
}

void validator_pop_type_info(Validator_T* v)
{
    if(v->exact_type_info_stack->size == 0)
    {
        v->need_exact_type_infos = true;
        return;
    }
    v->need_exact_type_infos = (bool) list_pop(v->exact_type_info_stack);
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

//    validate_semantics(&v, &queue);

    dbg_print_resolve_queue(&queue);

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

static void enter_typedef_scope(ASTObj_T* type_def, va_list args)
{
/*    if(type_def->data_type->kind == TY_ENUM)
        for(size_t i = 0; i < type_def->data_type->members->size; i++)
        {
            ASTObj_T* member = type_def->data_type->members->items[i];
            member->id->outer = type_def->id->outer;
        }*/
}

static void leave_typedef_scope(ASTObj_T* type_def, va_list args)
{
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
            [OBJ_TYPEDEF] = enter_typedef_scope,
            [OBJ_GLOBAL] = push_global_var,
            [OBJ_LOCAL] = register_local_var,
        },
        .obj_end_fns = {
            [OBJ_NAMESPACE] = leave_namespace_scope,
            [OBJ_FUNCTION] = leave_function_scope,
            [OBJ_GLOBAL] = pop_global_var,
            [OBJ_TYPEDEF] = leave_typedef_scope
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

static void validate_before_main_fn(Validator_T* v, ASTObj_T* fn)
{
    if(!v->ast->before_main)
    {
        v->ast->before_main = init_list();
        CONTEXT_ALLOC_REGISTER(v->context, v->ast->before_main);
    }

    list_push(v->ast->before_main, fn);
    fn->referenced = true;

    switch(fn->args->size)
    {
    case 0:
        break;
    case 2:
        {
            // TODO: implement this (argc/argv to [before_main])
            throw_error(v->context, ERR_INTERNAL, fn->tok, "`[before_main]` directive functions with 2 arguments are not supported yet");
        } break;
    default:
        {
            char* buf = calloc(BUFSIZ, sizeof(char));
            throw_error(v->context, ERR_TYPE_ERROR_UNCR, fn->tok, "`[before_main]` directive expects function `%s` to have 0 or 2 arguments, got %d", ast_id_to_str(buf, fn->id, BUFSIZ), fn->args->size);
            free(buf);
        } break;
    }
}

static void validate_after_main_fn(Validator_T* v, ASTObj_T* fn)
{
    if(!v->ast->after_main)
    {
        v->ast->after_main = init_list();
        CONTEXT_ALLOC_REGISTER(v->context, v->ast->before_main);
    }

    list_push(v->ast->after_main, fn);
    fn->referenced = true;

    switch(fn->args->size)
    {
    case 0:
        break;
    case 1:
        {
            ASTObj_T* arg = fn->args->items[0];
            if(unpack(arg->data_type)->kind != TY_I32)
                throw_error(v->context, ERR_TYPE_ERROR_UNCR, arg->tok, "`[after_main]` directive requires argument `%s` to be of type `i32`", arg->id->callee);
        } break;
    default:
        {
            char* buf = calloc(BUFSIZ, sizeof(char));
            throw_error(v->context, ERR_TYPE_ERROR_UNCR, fn->tok, "`[after_main]` directive expects function `%s` to have 0..1 arguments, got %d", ast_id_to_str(buf, fn->id, BUFSIZ), fn->args->size);
            free(buf);
        } break;
    }
}

static void validate_main_fn(Validator_T* v, ASTObj_T* fn)
{
    fn->is_entry_point = true;
    fn->referenced = true;
    v->ast->entry_point = fn;

    ASTType_T* return_type = unpack(fn->return_type);
    if(return_type->kind != TY_I32)
        throw_error(v->context, ERR_TYPE_ERROR_UNCR, fn->return_type->tok ? fn->return_type->tok : fn->tok, "expect type `i32` as return type for function `main`");

    switch(fn->args->size)
    {
    case 0:
        v->ast->mfk = MFK_NO_ARGS;
        break;
    case 1:
        v->ast->mfk = MFK_ARGV_PTR;
        break;
    case 2:
        v->ast->mfk = MFK_ARGC_ARGV_PTR;
        break;
    default:
        throw_error(v->context, ERR_UNDEFINED_UNCR, fn->tok, "expect 0 or 2 arguments for function `main`, got %ld", fn->args->size);
        return;
    }
}

static bool stmt_returns_value(Validator_T* v, ASTNode_T* node)
{
    switch(node->kind)
    {
        case ND_RETURN:
            return true;
        case ND_CALL:
            return node->expr->data_type->no_return;
        case ND_BLOCK:
            for(size_t i = 0; i < node->stmts->size; i++)
            {
                if(stmt_returns_value(v, node->stmts->items[i]))
                {
                    if(node->stmts->size - i > 1)
                        throw_error(v->context, ERR_UNREACHABLE, ((ASTNode_T*) node->stmts->items[i + 1])->tok, "unreachable code after return statement");
                    return true;
                }
            }
            return false;
        case ND_IF:
            return stmt_returns_value(v, node->if_branch) && node->else_branch ? stmt_returns_value(v, node->else_branch) : false;
        case ND_LOOP:
        case ND_FOR:
        case ND_WHILE:
        case ND_DO_WHILE:
            return stmt_returns_value(v, node->body);
        case ND_MATCH:
            if(!node->default_case)
                return false;
            
            {
                u64 cases_return = 0;
                for(size_t i = 0; i < node->cases->size; i++)
                    if(stmt_returns_value(v, ((ASTNode_T*) node->cases->items[i])->body))
                        cases_return++;
                
                return cases_return == node->cases->size && stmt_returns_value(v, node->default_case->body);
            }
        case ND_FOR_RANGE:
            return stmt_returns_value(v, node->body);
        case ND_EXPR_STMT:
            return stmt_returns_value(v, node->expr);
        default: 
            return false;
    }
}

static void validate_function(Validator_T* v, ASTObj_T* fn)
{
    if(fn->after_main)
        validate_after_main_fn(v, fn);

    if(fn->before_main)
        validate_before_main_fn(v, fn);

    if(strcmp(fn->id->callee, "main") == 0 && fn->id->outer == NULL)
    {
        v->main_function_found = true;
        validate_main_fn(v, fn);
    }

    validator_pop_obj(v);
}

static void validate_data_type(ASTType_T* type, va_list args)
{
    GET_VALIDATOR(args);
    
    ASTType_T* unpacked = unpack(type);

    type->size = unpacked->size = get_type_size(v, unpacked);
    type->align = unpacked->align = get_type_align(v, unpacked);
}

static void validate_semantics(Validator_T* v, ResolveQueue_T* queue)
{
    ResolveQueueNode_T* node = queue->head;
    while(node)
    {
        if(node->obj->data_type)
        {
            static const ASTIteratorList_T iter = {
                .type_end = validate_data_type
            };
            ast_iterate_type(&iter, node->obj->data_type);
        }
        
        if(node->method & RESOLVE_DEEP)
        {
            switch(node->obj->kind)
            {
                
            }
        }
        node = node->next;
    }
}

static i32 get_type_size_impl(Validator_T* v, ASTType_T* type, List_T** struct_types);

static i32 get_union_size(ASTType_T* u_type)
{
    i32 biggest = 0;
    for(size_t i = 0; i < u_type->members->size; i++)
    {
        ASTNode_T* member = u_type->members->items[i];
        if(member->data_type->size > biggest)
            biggest = member->data_type->size;
    }
    return biggest;
}

static i32 get_struct_size(Validator_T* v, ASTType_T* s_type, List_T** struct_types)
{
    if(!*struct_types)
        *struct_types = init_list();

    if(list_contains(*struct_types, s_type)) {
        char buf[BUFSIZ] = {};
        throw_error(v->context, ERR_TYPE_ERROR, s_type->tok, "detected recursive structs, encountered `%s` twice.", ast_type_to_str(v->context, buf, s_type, LEN(buf)));
        return 0;
    }

    list_push(*struct_types, s_type);

    i64 bits = 0;
    for(size_t i = 0; i < s_type->members->size; i++)
    {
        ASTNode_T* member = s_type->members->items[i];
        member->data_type->size = get_type_size_impl(v, member->data_type, struct_types);
        bits = align_to(bits, get_type_align(v, member->data_type) * 8);
        member->offset = bits / 8;
        bits += member->data_type->size * 8;
    }

    list_pop(*struct_types);

    return align_to(bits, get_type_align(v, s_type) * 8) / 8;
};

static i32 get_type_size_impl(Validator_T* v, ASTType_T* type, List_T** struct_types)
{
    switch(type->kind)
    {
        case TY_I8:
            return I8_S;
        case TY_U8:
            return U8_S;
        case TY_CHAR:
            return CHAR_S;
        case TY_BOOL:
            return BOOL_S;
        case TY_I16:
            return I16_S;
        case TY_U16:
            return U16_S;
        case TY_I32:
            return I32_S;
        case TY_U32:
            return U32_S;
        case TY_ENUM:
            return ENUM_S;
        case TY_I64:
            return I64_S;
        case TY_U64:
            return U64_S;
        case TY_F32:
            return F32_S;
        case TY_F64:
            return F64_S;
        case TY_F80:
            return F80_S;
        case TY_VOID:
            return VOID_S;
        case TY_PTR:
        case TY_FN:
            return PTR_S;
        case TY_TYPEOF:
            return get_type_size_impl(v, expand_typedef(v, type->num_indices_node->data_type), struct_types);
        case TY_UNDEF:
            return get_type_size_impl(v, expand_typedef(v, type), struct_types);
        case TY_C_ARRAY:
            if(type->num_indices == 0)
                return 0;
            return get_type_size_impl(v, type->base, struct_types) * type->num_indices;
        case TY_VLA:
            return PTR_S;
        case TY_ARRAY:
                return get_type_size_impl(v, type->base, struct_types) * type->num_indices + PTR_S;
        case TY_STRUCT:
            if(type->is_union)
                return get_union_size(type);
            else
                return get_struct_size(v, type, struct_types);
        default:
            return 0;
    }
    
    throw_error(v->context, ERR_TYPE_ERROR, type->tok, "could not resolve data type size");
    return 0;
}

static i32 get_type_size(Validator_T* v, ASTType_T* type)
{
    List_T* struct_types = NULL;
    i32 size = get_type_size_impl(v, type, &struct_types);
    if(struct_types)
        free_list(struct_types);
    return size;
}

static i32 get_type_align(Validator_T* v, ASTType_T* type)
{
    switch(type->kind)
    {
        case TY_C_ARRAY:
        case TY_PTR:
            return MAX(pow(2, floor(log(type->base->size)/log(2))), 8);
        default:
            return MAX(pow(2, floor(log(type->size)/log(2))), 1);
    }
}

