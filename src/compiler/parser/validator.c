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
#include "ast/types.h"
#include "parser/queue.h"
#include "parser/typechecker.h"
#include "timer/timer.h"
#include "util.h"
#include "codegen/backend.h"

#include <asm-generic/errno-base.h>
#include <assert.h>
#include <ctype.h>
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

static void validate_drop_functions(Validator_T* v);
static void validate_semantics(Validator_T* v, ResolveQueue_T* queue);

// validator struct functions
static void init_validator(Validator_T* v, Context_T* context, ASTProg_T* ast)
{
    memset(v, 0, sizeof(struct VALIDATOR_STRUCT));
    v->context = context;
    v->ast = ast;
    v->obj_stack = init_list();
    v->exact_type_info_stack = init_list();
    typechecker_init(&v->typechecker, context);
    init_constexpr_resolver(&v->constexpr_resolver, context, ast);
}

static void free_validator(Validator_T* v)
{
    free_constexpr_resolver(&v->constexpr_resolver);
    typechecker_free(&v->typechecker);
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

static ASTObj_T* in_function(Validator_T* v)
{
    if(!v->current_obj)
        return NULL;

    if(v->current_obj->kind == OBJ_FUNCTION || v->current_obj->kind == OBJ_LAMBDA)
        return v->current_obj;

    for(ssize_t i = v->obj_stack->size - 1; i >= 0; i--)
    {
        ASTObj_T* obj = v->obj_stack->items[i];
        if(obj->kind == OBJ_FUNCTION || obj->kind == OBJ_LAMBDA)
            return obj;
    }
    return NULL;
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
    
    // dbg_print_resolve_queue(&queue); 

    validate_semantics(&v, &queue);
    resolve_queue_free(&queue);

    end_scope(&v);
    assert(v.scope_depth == 0);

    // end the validator
    v.global_scope = NULL;
    v.current_obj = NULL;

    // check for the main function
    validate_drop_functions(&v);
    if(!v.main_function_found && context->flags.require_entrypoint)
    {
        LOG_ERROR(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " missing entrypoint; no `main` function declared.\n");
       context->emitted_errors++;
    }

finish:
    free_validator(&v);
    timer_stop(context);

    return context->emitted_errors;
}

#undef RETURN_IF_ERRORED

static ASTExitFnHandle_T* find_drop_function(Validator_T* v, ASTType_T* type)
{
    if(!v->ast->type_exit_fns)
        return NULL;

    for(size_t i = 0; i < v->ast->type_exit_fns->size; i++)
    {
        ASTExitFnHandle_T* handle = v->ast->type_exit_fns->items[i];
        if(types_equal(v->context, handle->type, type))
            return handle;
    }
    return NULL;
}

static void validate_drop_functions(Validator_T* v)
{
    if(!v->ast->type_exit_fns)
        return;
    for(size_t i = 0; i < v->ast->type_exit_fns->size; i++)
    {
        ASTExitFnHandle_T* handle = v->ast->type_exit_fns->items[i];
        ASTObj_T* fn = handle->fn;

        ASTExitFnHandle_T* found = find_drop_function(v, handle->type);
        if(handle != found)
        {
            char* buf = malloc(BUFSIZ * sizeof(char));
            *buf = '\0';
            throw_error(v->context, ERR_REDEFINITION_UNCR, handle->tok, "exit function for data type `%s` already defined", ast_type_to_str(v->context, buf, handle->type, BUFSIZ));
            free(buf);
        }

        if(fn->args->size != 1)
            throw_error(v->context, ERR_TYPE_ERROR_UNCR, handle->tok, "exit function must have one argument");

        
        if(!types_equal(v->context, ((ASTObj_T*) fn->args->items[0])->data_type, handle->type))
            throw_error(v->context, ERR_TYPE_ERROR_UNCR, handle->tok, "specified data type and first argument type of function `%s` do not match", fn->id->callee);
    }
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

static void enter_for_scope(ASTNode_T* block, va_list args)
{
    GET_VALIDATOR(args);
    begin_scope(v, NULL);
}

static void leave_for_scope(ASTNode_T* block, va_list args)
{
    GET_VALIDATOR(args);
    end_scope(v);
}

static void enter_with_scope(ASTNode_T* block, va_list args)
{
    GET_VALIDATOR(args);
    begin_scope(v, NULL);
}

static void leave_with_scope(ASTNode_T* block, va_list args)
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

    if(node->referenced_obj->kind == OBJ_ENUM_MEMBER && node->id->outer)
    {
        IdentResolveResult_T result = scope_resolve_ident(v, v->current_scope, node->id->outer);
        assert(result.found);

        node->id->outer->outer = result.obj->id->outer;
    }

    if(v->current_obj && v->current_obj->constexpr && !node->referenced_obj->constexpr && node->referenced_obj->kind != OBJ_FN_ARG)
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
            [ND_FOR] = enter_for_scope,
            [ND_WITH] = enter_with_scope,
            [ND_LAMBDA] = enter_lambda_scope,
            [ND_USING] = enter_using_scope,
        },
        .node_end_fns = {
            [ND_BLOCK] = leave_block_scope,
            [ND_FOR] = leave_for_scope,
            [ND_WITH] = leave_with_scope,
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

static bool is_number(ASTType_T* type)
{
    if(!type)
        return false;

    switch(unpack(type)->kind)
    {
        case TY_I8:
        case TY_I16:
        case TY_I32:
        case TY_I64:
        case TY_U8:
        case TY_U16: 
        case TY_U32:
        case TY_U64:
        case TY_F32:
        case TY_F64:
        case TY_F80:
        case TY_CHAR:
        case TY_ENUM: // enums get at the moment treated as numbers to support operations
            return true;

        default:
            return false;
    }
}

static bool is_ptr(ASTType_T* type)
{
    return type && unpack(type)->kind == TY_PTR;
}

static bool is_bool(ASTType_T* type)
{
    return type && unpack(type)->kind == TY_BOOL;
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

static void validate_return_type(Validator_T* v, ASTType_T* type, Token_T* tok)
{
    if(type->kind == TY_C_ARRAY || type->kind == TY_ARRAY)
    {
        char* buf = malloc(BUFSIZ * sizeof(char));
        *buf = '\0';
        throw_error(v->context, ERR_TYPE_ERROR_UNCR, EITHER(type->tok, tok), "functions cannot return type `%s`", ast_type_to_str(v->context, buf, type, BUFSIZ));
        free(buf);
    }
}

static u64 calculate_array_size(Validator_T* v, ASTType_T* type)
{
    if(type->num_indices)
        return type->num_indices;
    ASTNode_T* result = eval_constexpr(&v->constexpr_resolver, type->num_indices_node);
    type->num_indices = const_u64(v->context, result);
    return type->num_indices;
}

static void validate_data_type(Validator_T* v, ASTType_T* type)
{
    ASTType_T* unpacked = unpack(type);

    if(type->kind == TY_ARRAY || type->kind == TY_C_ARRAY)
        calculate_array_size(v, type);

    type->size = unpacked->size = get_type_size(v, unpacked);
    type->align = unpacked->align = get_type_align(v, unpacked);

    if(type->kind == TY_FN)
        validate_return_type(v, unpack(type->base), type->tok);
}

static void validate_obj_shallow(Validator_T* v, ASTObj_T* obj)
{
    switch(obj->kind)
    {
    case OBJ_FUNCTION:
        validate_data_type(v, obj->data_type);
        break;

    case OBJ_GLOBAL:
        validate_data_type(v, obj->data_type);
        break;

    case OBJ_ENUM_MEMBER:
        validate_data_type(v, obj->data_type);
        break;

    case OBJ_TYPEDEF:
        break;

    default:
        unreachable();
        break;
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

/* FIXME:    if(v->context->ct == CT_ASM && fn->return_type->size > 16)
    {
        fn->return_ptr = init_ast_obj(&v->context->raw_allocator, OBJ_LOCAL, fn->return_type->tok);
        fn->return_ptr->data_type = init_ast_type(&v->context->raw_allocator, TY_PTR, fn->return_type->tok);
        fn->return_ptr->data_type->base = fn->return_type;
        fn->return_ptr->data_type->size = get_type_size(v, fn->return_ptr->data_type);
        fn->return_ptr->data_type->align = 8;
    }*/

    if(unpack(fn->return_type)->kind != TY_VOID && !fn->is_extern && !stmt_returns_value(v, fn->body))
        throw_error(v->context, ERR_NORETURN, fn->tok, "function `%s` does not return a value", fn->id->callee);

    for(size_t i = 0; i < fn->args->size; i++)
    {
        ASTObj_T* arg = fn->args->items[i];
        if(!arg->referenced && !fn->is_extern && !fn->ignore_unused && arg->id->callee[0] != '_')
            throw_error(v->context, ERR_UNUSED, arg->tok, "unused function argument `%s`, prefix with `_` to disable this warning", arg->id->callee);
    }

    if(v->scope_depth == 1 && strcmp(fn->id->callee, "_start") == 0)
        throw_error(v->context, ERR_MISC, fn->id->tok, "cannot name a function \"_start\" in global scope");
}

static void iter_enter_function(ASTObj_T* fn, va_list args)
{
    GET_VALIDATOR(args);
    validator_push_obj(v, fn);
}

static void iter_leave_function(ASTObj_T* fn, va_list args)
{
    GET_VALIDATOR(args);
    validate_function(v, fn);
    validator_pop_obj(v);

    typecheck_obj(&v->typechecker, fn);
}

static void iter_enter_typedef(ASTObj_T* type_def, va_list args)
{
    GET_VALIDATOR(args);
    validator_push_obj(v, type_def);
    if(type_def->data_type->kind == TY_ENUM)
        for(size_t i = 0; i < type_def->data_type->members->size; i++)
        {
            ASTObj_T* member = type_def->data_type->members->items[i];
            member->id->outer = type_def->id;
        }
}

static void iter_leave_typedef(ASTObj_T* type_def, va_list args)
{
    GET_VALIDATOR(args);
    validator_pop_obj(v);

    typecheck_obj(&v->typechecker, type_def);
}

static void iter_enter_fn_arg(ASTObj_T* arg, va_list args)
{
    GET_VALIDATOR(args);

    if(v->current_obj->constexpr)
        arg->constexpr = true;

    validator_push_obj(v, arg);
}

static void iter_leave_fn_arg(ASTObj_T* arg, va_list args)
{
    GET_VALIDATOR(args);

    ASTType_T* type = unpack(arg->data_type);
    if(arg->data_type->is_constant || type->is_constant)
        arg->is_constant = true;

    switch(type->kind)
    {
        case TY_VOID:
            throw_error(v->context, ERR_TYPE_ERROR, arg->tok, "`void` type is not allowed for function arguments");
            break;
        
        case TY_ARRAY:
            throw_error(v->context, ERR_TYPE_ERROR, arg->tok, "array type is not allowed for function arguments, use VLA `[]`");
            break;
        
        case TY_C_ARRAY:
            throw_error(v->context, ERR_TYPE_ERROR, arg->tok, "c-array type is not allowed for function arguments, use pointer `&`");
            break;
        
        default:
            break;
    }

    validator_pop_obj(v);
}

static void iter_enter_global(ASTObj_T* global, va_list args)
{
    if(global->value)
        global->value->is_assigning = true;
}

static void iter_leave_global(ASTObj_T* global, va_list args)
{
    GET_VALIDATOR(args);

    if(global->value)
        global->value = eval_constexpr(&v->constexpr_resolver, global->value);

    if(!global->data_type)
    {
        if(!global->value->data_type)
        {
            throw_error(v->context, ERR_TYPE_ERROR, global->value->tok, "could not resolve datatype for `%s`", global->id->callee);
            return;
        }

        global->data_type = global->value->data_type;
    }

    ASTType_T* type = unpack(global->data_type);
    if(type->is_constant)
        global->is_constant = true;
    if(global->is_constant)
        global->constexpr = true;

    switch(type->kind)
    {
    case TY_VOID:
        throw_error(v->context, ERR_TYPE_ERROR, global->tok, "`void` type is not allowed for variables"); 
        break;
    
    case TY_VLA:
        if(global->value && unpack(global->value->data_type)->kind == TY_ARRAY)
        {
            global->data_type->kind = TY_ARRAY;
            global->data_type->num_indices = unpack(global->value->data_type)->num_indices;
        }
        break;
    
    default:
        break;
    }

    typecheck_obj(&v->typechecker, global);
}

static void iter_enter_enum_member(ASTObj_T* member, va_list args)
{
    GET_VALIDATOR(args);
    validator_push_obj(v, member);
}

static void iter_leave_enum_member(ASTObj_T* member, va_list args)
{
    GET_VALIDATOR(args);

// FIXME:    member->value = eval_constexpr(&v->constexpr_resolver, member->value);

    validator_pop_obj(v);

    typecheck_obj(&v->typechecker, member);
}

static void iter_enter_block(ASTNode_T* block, va_list args)
{
    GET_VALIDATOR(args);

    for(size_t i = 0; i < block->locals->size; i++)
    {
        ASTObj_T* var = block->locals->items[i];
        var->constexpr = v->current_obj->constexpr;
    }
}

static void iter_leave_block(ASTNode_T* block, va_list args)
{
    GET_VALIDATOR(args);

    for(size_t i = 0; i < block->locals->size; i++)
    {
        ASTObj_T* var = block->locals->items[i];
        if(!var->referenced && !v->current_obj->ignore_unused)
            throw_error(v->context, ERR_UNUSED, var->tok, "unused local variable `%s`", var->id->callee);
    }
}

static void iter_validate_return(ASTNode_T* ret, va_list args)
{
    GET_VALIDATOR(args);

    if(!in_function(v))
    {
        throw_error(v->context, ERR_SYNTAX_ERROR, ret->tok, "unexpected return statement outside of function");
        return;
    }
}

static void iter_validate_for_range(ASTNode_T* for_range, va_list args)
{
    GET_VALIDATOR(args);
    ASTType_T* left = unpack(for_range->left->data_type);

    if(!is_integer(left))
    {
        char* buf = malloc(BUFSIZ * sizeof(char));
        *buf = '\0';
        throw_error(v->context, ERR_TYPE_ERROR_UNCR, for_range->left->tok, "expression of range-based for loop expected to be integer, got %s", ast_type_to_str(v->context, buf, left, BUFSIZ));
        free(buf);
    }

    ASTType_T* right = unpack(for_range->right->data_type);
    if(!is_integer(right))
    {
        char* buf = malloc(BUFSIZ * sizeof(char));
        *buf = '\0';
        throw_error(v->context, ERR_TYPE_ERROR_UNCR, for_range->right->tok, "expression of range-based for loop expected to be integer, got %s", ast_type_to_str(v->context, buf, right, BUFSIZ));
        free(buf);
    }
}

static void iter_validate_match_type(ASTNode_T* match, va_list args)
{
    GET_VALIDATOR(args);
    for(size_t i = 0; i < match->cases->size; i++)
    {
        ASTNode_T* case_stmt = match->cases->items[i];

        if(types_equal(v->context, match->data_type, case_stmt->data_type)) 
        {
            match->body = case_stmt->body;
            return;
        }
    }

    if(match->default_case)
        match->body = match->default_case->body;
}

static void iter_validate_with(ASTNode_T* with, va_list args)
{
    GET_VALIDATOR(args);

    if(!with->condition->data_type)
        throw_error(v->context, ERR_TYPE_ERROR, with->obj->tok, "could not resolve data type for `%s`", with->obj->id->callee);
    
    ASTExitFnHandle_T* handle = find_drop_function(v, with->condition->data_type);
    if(!handle)
    {
        char* buf = malloc(BUFSIZ * sizeof(char));
        *buf = '\0';
        throw_error(v->context, ERR_TYPE_ERROR_UNCR, with->obj->tok, "type `%s` does not have a registered exit function.\nRegister one by using the `exit_fn` compiler directive",
            ast_type_to_str(v->context, buf, with->condition->data_type, BUFSIZ));
        free(buf);
    }
    with->exit_fn = handle->fn;
}

static void iter_validate_case(ASTNode_T* c_stmt, va_list args)
{
    GET_VALIDATOR(args);

    if(c_stmt->condition && c_stmt->mode != ND_EQ && !is_number(c_stmt->condition->data_type)) {
        char* buf = malloc(BUFSIZ * sizeof(char));
        *buf = '\0';
        throw_error(v->context, ERR_TYPE_ERROR_UNCR, c_stmt->tok, "match cases with special ranges are only supported with number types, got `%s`",
            ast_type_to_str(v->context, buf, c_stmt->condition->data_type, BUFSIZ));
        free(buf);
    }
}

static void iter_validate_expr_stmt(ASTNode_T* expr_stmt, va_list args)
{
    expr_stmt->expr->result_ignored = expr_stmt->expr->kind == ND_ASSIGN;
}

static void iter_validate_extern_c_block(ASTNode_T* block, va_list args)
{
    GET_VALIDATOR(args);
    if(strcmp(v->context->backend->name, "c99") != 0)
        throw_error(v->context, ERR_SYNTAX_ERROR_UNCR, block->tok, "`extern \"C\"` statements are only supported with the C99 backend.\nUse `--backend c99` to use this backend.");
}

static void iter_validate_call(ASTNode_T* call, va_list args)
{
    GET_VALIDATOR(args);
    
    ASTType_T* call_type = unpack(call->expr->data_type);
    switch(call_type->kind)
    {
    case TY_FN:
        if(call_type->base)
            call->data_type = call_type->base;
        else
            call->data_type = (ASTType_T*) primitives[TY_VOID];
        call->called_obj = call->expr->referenced_obj;
        break;
    default:
        {
            char* buf = malloc(BUFSIZ * sizeof(char));
            *buf = '\0';
            throw_error(v->context, ERR_TYPE_ERROR, call->tok, "cannot call expression of data type `%s`", ast_type_to_str(v->context, buf, call_type, BUFSIZ));
            free(buf);
        } break;
    }

    if(call->expr->kind == ND_ID)
        call->expr->call = call;

    size_t expected_arg_num = call_type->arg_types->size;
    size_t received_arg_num = call->args->size;

    if(is_variadic(call_type) && received_arg_num < expected_arg_num)
    {
        char* buf = malloc(BUFSIZ * sizeof(char));
        *buf = '\0';
        throw_error(v->context, ERR_CALL_ERROR_UNCR, call->tok, "type `%s` expects %lu or more call arguments, got %lu", ast_type_to_str(v->context, buf, call_type, BUFSIZ), expected_arg_num, received_arg_num);
        free(buf);
    }
    else if(!is_variadic(call_type) && received_arg_num != expected_arg_num)
    {
        char* buf = malloc(BUFSIZ * sizeof(char));
        *buf = '\0';
        throw_error(v->context, ERR_CALL_ERROR_UNCR, call->tok, "type `%s` expects %lu call arguments, got %lu", ast_type_to_str(v->context, buf, call_type, BUFSIZ), expected_arg_num, received_arg_num);
        free(buf);
    }

    // if we compile using the assembly compiler, a buffer for the return value is needed when handling big structs
    /* FIXME: if(v->context->ct == CT_ASM && call->data_type && unpack(call->data_type)->kind == TY_STRUCT)
    {
        ASTObj_T* ret_buf = init_ast_obj(&v->context->raw_allocator, OBJ_LOCAL, call->tok);
        ret_buf->data_type = call->data_type;
        
        ASTObj_T* fn = in_function(v);
        if(fn) {
            list_push(fn->objs, ret_buf);
            call->return_buffer = ret_buf;
        }
        else {
            // TODO
        }
    }*/
}

static void iter_validate_identifier(ASTNode_T* id, va_list args)
{
    GET_VALIDATOR(args);

    assert(id->referenced_obj != NULL);

    switch(id->referenced_obj->kind)
    {
        case OBJ_GLOBAL:
        case OBJ_FUNCTION:
        case OBJ_ENUM_MEMBER:
            break;
        
        case OBJ_LOCAL:
        case OBJ_FN_ARG:
            id->referenced_obj->referenced = true;
            break;

        default:
            throw_error(v->context, ERR_TYPE_ERROR, id->id->tok, 
                "identifier `%s` is of kind %s, expect variable or function name", 
                id->id->callee, obj_kind_to_str(id->referenced_obj->kind)
            );
            return;
    }

    id->data_type = id->referenced_obj->data_type;
}

static void iter_validate_closure(ASTNode_T* closure, va_list args)
{
    closure->data_type = ((ASTNode_T*) closure->exprs->items[closure->exprs->size - 1])->data_type;
}

static void iter_validate_reference(ASTNode_T* ref, va_list args)
{
    GET_VALIDATOR(args);
    if(!ref->data_type)
    {
        ref->data_type = init_ast_type(&v->context->raw_allocator, TY_PTR, ref->tok);
        ref->data_type->base = ref->right->data_type;
    }
}

static void iter_validate_dereference(ASTNode_T* deref, va_list args)
{
    GET_VALIDATOR(args);

    ASTType_T* right_dt = unpack(deref->right->data_type);
    if(right_dt->kind != TY_PTR && right_dt->kind != TY_C_ARRAY)
    {
        throw_error(v->context, ERR_TYPE_ERROR, deref->tok, "can only dereference variables with pointer type");
        return;
    }

    deref->data_type = right_dt->base;
}

static ASTNode_T* find_member_in_struct(Validator_T* v, ASTType_T* type, ASTNode_T* id)
{
    if(id->kind != ND_ID)
        return NULL;

    bool is_ptr = false;
    while(type->kind == TY_PTR)
    {
        if(!is_ptr)
        {
            is_ptr = true;
            type = type->base;
        }
        else
            break;
    }

    type = unpack(type);
    if(type->kind != TY_STRUCT)
    {
        char* buf = malloc(BUFSIZ * sizeof(char));
        *buf = '\0';
        throw_error(v->context, ERR_TYPE_ERROR, id->tok, "cannot get member of type `%s`", ast_type_to_str(v->context, buf, type, BUFSIZ));
        free(buf);
        return NULL;
    }

    for(size_t i = 0; i < type->members->size; i++)
    {
        ASTNode_T* member = type->members->items[i];
        if(strcmp(member->id->callee, id->id->callee) == 0)
            return member;
    }

    return NULL;
}

static void iter_validate_member(ASTNode_T* member, va_list args)
{
    GET_VALIDATOR(args);

    if(!member->left->data_type)
    {
        throw_error(v->context, ERR_TYPE_CAST_WARN, member->left->tok, "could not resolve data type for `%s`", member->right->id->callee);
        return;
    }

    ASTNode_T* found = find_member_in_struct(v, unpack(member->left->data_type), member->right);
    if(!found)
    {
        char* buf = malloc(BUFSIZ * sizeof(char));
        *buf = '\0';
        throw_error(v->context, ERR_TYPE_ERROR, member->tok, "type `%s` has no member named `%s`", ast_type_to_str(v->context, buf, member->left->data_type, BUFSIZ), member->right->id->callee);
        free(buf);
        return;
    }

    member->data_type = found->data_type;
    member->body = found;

    if(is_ptr(member->left->data_type))
    {
        // convert x->y to (*x).y
        ASTNode_T* new_left = init_ast_node(&v->context->raw_allocator, ND_DEREF, member->left->tok);
        new_left->data_type = member->left->data_type->base;
        new_left->right = member->left;
        member->left = new_left;
    }
}

static void iter_validate_binop(ASTNode_T* op, va_list args)
{
    GET_VALIDATOR(args);

    if(!is_number(op->left->data_type) && !is_ptr(op->left->data_type))
    {
        throw_error(v->context, ERR_TYPE_ERROR, op->tok, "left: expect integer or pointer type");
        return;
    }

    if(!is_number(op->right->data_type) && !is_ptr(op->right->data_type))
    {
        throw_error(v->context, ERR_TYPE_ERROR, op->tok, "right: expect integer or pointer type");
        return;
    }

    size_t left_size = get_type_size(v, op->left->data_type);
    size_t right_size = get_type_size(v, op->right->data_type);

    if(left_size < right_size)
        op->data_type = op->right->data_type;
    else
        op->data_type = op->left->data_type;
}

static void iter_validate_modulo(ASTNode_T* mod, va_list args)
{
    GET_VALIDATOR(args);

    if(!is_integer(unpack(mod->left->data_type)))
        throw_error(v->context, ERR_TYPE_ERROR_UNCR, mod->tok, "left: expect integer type for modulo operation");

    if(!is_integer(unpack(mod->right->data_type)))
        throw_error(v->context, ERR_TYPE_ERROR_UNCR, mod->tok, "right: expect integer type for modulo operation");

    mod->data_type = mod->left->data_type;
}

static void iter_validate_negation(ASTNode_T* neg, va_list args)
{
     GET_VALIDATOR(args);

    if(!is_number(neg->right->data_type))
        throw_error(v->context, ERR_TYPE_ERROR_UNCR, neg->tok, "can only do bitwise operations on integer types");

    // TODO: change type of unsigned integers
    neg->data_type = neg->right->data_type;
}

static void iter_validate_bitwise_negation(ASTNode_T* neg, va_list args)
{
    GET_VALIDATOR(args);

    if(!is_integer(unpack(neg->right->data_type)))
        throw_error(v->context, ERR_TYPE_ERROR_UNCR, neg->tok, "expect integer type for bitwise negation");

    neg->data_type = neg->right->data_type;
}

static void iter_validate_not(ASTNode_T* not, va_list args)
{
    not->data_type = (ASTType_T*) primitives[TY_BOOL];
}

static void iter_validate_equals(ASTNode_T* equals, va_list args)
{
    equals->data_type = (ASTType_T*) primitives[TY_BOOL];
}

static void iter_validate_lt_gt(ASTNode_T* op, va_list args)
{
    GET_VALIDATOR(args);

    if(!is_number(op->left->data_type) && !is_ptr(op->left->data_type))
    {
        throw_error(v->context, ERR_TYPE_ERROR, op->tok, "left: expect integer or pointer type");
        return;
    }

    if(!is_number(op->right->data_type) && !is_ptr(op->right->data_type))
    {
        throw_error(v->context, ERR_TYPE_ERROR, op->tok, "right: expect integer or pointer type");
        return;
    }

    op->data_type = (ASTType_T*) primitives[TY_BOOL];
}

static void iter_validate_and_or(ASTNode_T* op, va_list args)
{
    op->data_type = op->left->data_type;
}

static void iter_validate_bitwise_op(ASTNode_T* op, va_list args)
{
    GET_VALIDATOR(args);

    if(!is_integer(unpack(op->left->data_type)))
    {
        throw_error(v->context, ERR_TYPE_ERROR, op->tok, "left: can only do bitwise operations on integer types");
        return;
    }

    if(!is_integer(unpack(op->right->data_type)))
    {
        throw_error(v->context, ERR_TYPE_ERROR, op->tok, "right: can only do bitwise operations with integer types");
        return;
    }

    op->data_type = op->right->data_type->size > op->left->data_type->size ? op->right->data_type : op->left->data_type;
}

static void iter_validate_inc_dec(ASTNode_T* op, va_list args)
{
    GET_VALIDATOR(args);

    if(!is_number(op->left->data_type) && !is_ptr(op->left->data_type))
    {
        throw_error(v->context, ERR_TYPE_ERROR, op->tok, "expect a number type");
        return;
    }

    op->data_type = op->left->data_type;
}

static bool indexable(ASTTypeKind_T tk)
{
    return tk == TY_VLA || tk == TY_ARRAY || tk == TY_C_ARRAY || tk == TY_PTR;
}

static void iter_validate_index(ASTNode_T* index, va_list args)
{
    GET_VALIDATOR(args);

    ASTType_T* left_type = unpack(index->left->data_type);
    assert(left_type != NULL);

    if(!indexable(left_type->kind))
    {
        char buf[BUFSIZ];
        throw_error(v->context, ERR_TYPE_ERROR, index->tok, "cannot get an index value from type `%s`", ast_type_to_str(v->context, buf, left_type, BUFSIZ));
        return;
    }

    if(!is_integer(unpack(index->expr->data_type)))
    {
        throw_error(v->context, ERR_TYPE_ERROR, index->tok, "expect an integer type for the index operator");
        return;
    }
    index->data_type = left_type->base;

    if(index->from_back)
    {
        if(left_type->kind != TY_C_ARRAY && left_type->kind != TY_ARRAY && left_type->kind != TY_VLA)
        {
            char buf[BUFSIZ] = {'\0'};
            throw_error(v->context, ERR_TYPE_ERROR, index->tok, "cannot get reverse index of type `%s`, need fixed-size array", ast_type_to_str(v->context, buf, index->left->data_type, LEN(buf)));
        }
    }
}

static ASTNode_T* expand_closure(ASTNode_T* cl)
{
    return cl->kind == ND_CLOSURE ? expand_closure(cl->exprs->items[cl->exprs->size - 1]) : cl;
}

static void local_initializer(Validator_T* v, ASTNode_T* assign, ASTObj_T* local)
{
     if(!local->data_type)
    {
        if(!assign->right->data_type)
        {
            throw_error(v->context, ERR_TYPE_ERROR, local->id->tok, "could not resolve datatype for `%s`", local->id->callee);
            return;
        }
        local->data_type = assign->right->data_type;
    }
    ASTType_T* expanded = unpack(local->data_type);

    if(expanded->kind == TY_VOID)
        throw_error(v->context, ERR_TYPE_ERROR, local->tok, "`void` type is not allowed for variables");
    
    if(local->data_type->is_constant)
        local->is_constant = true;
    
    assign->left->data_type = local->data_type;
    local->data_type->size = get_type_size(v, local->data_type);
}

static void iter_enter_assignment(ASTNode_T* assign, va_list args)
{
    if(assign->right->kind == ND_ARRAY || assign->right->kind == ND_STRUCT)
        assign->right->is_assigning = true;
}

static void iter_leave_assignment(ASTNode_T* assign, va_list args)
{
    GET_VALIDATOR(args);

    // if the assignments initializes a local variable, resolve the variables type
    if(assign->is_initializing && assign->referenced_obj)
        local_initializer(v, assign, assign->referenced_obj);


    switch(expand_closure(assign->left)->kind)
    {
        case ND_MEMBER:
        case ND_INDEX:
        case ND_DEREF:
        case ND_REF:
            // todo: implement checking for constant types
            break;
        case ND_ID: 
        {
            ASTObj_T* assigned_obj = assign->left->referenced_obj;
            if(!assigned_obj)
                return;

            switch(assigned_obj->kind)
            {
                case OBJ_GLOBAL:
                case OBJ_LOCAL:
                case OBJ_FN_ARG:
                    //if(assigned_obj->is_constant)
                    //    throw_error(v->context, ERR_CONST_ASSIGN, assigned_obj->tok, "cannot assign a value to constant %s `%s`", obj_kind_to_str(assigned_obj->kind), assigned_obj->id->callee);
                    if(assigned_obj->is_constant)
                        assigned_obj->data_type->is_constant = true;
                    break;

                case OBJ_FUNCTION:
                case OBJ_NAMESPACE:
                case OBJ_ENUM_MEMBER:
                case OBJ_TYPEDEF:
                default:
                    throw_error(v->context, ERR_MISC, assign->tok, "cannot assign value to %s `%s`", obj_kind_to_str(assigned_obj->kind), assigned_obj->id->callee);
            }
        } break;

        default:
            throw_error(v->context, ERR_MISC, assign->left->tok, "cannot assign value to `%s`", assign->left->tok->value);
    }

    assign->data_type = assign->left->data_type;

    if(unpack(assign->data_type)->kind == TY_VOID)
        throw_error(v->context, ERR_TYPE_ERROR, assign->tok, "cannot assign type `void`");
}

static void validate_anon_struct_lit(Validator_T* v, ASTNode_T* struct_lit)
{
    if(!struct_lit->args->size)
    {
        throw_error(v->context, ERR_TYPE_ERROR, struct_lit->tok, "cannot resolve data type of empty anonymous struct literal `{}`");
        return;
    }

    ASTType_T* type = init_ast_type(&v->context->raw_allocator, TY_STRUCT, struct_lit->tok);
    CONTEXT_ALLOC_REGISTER(v->context, type->members = init_list());

    for(size_t i = 0; i < struct_lit->args->size; i++)
    {
        ASTNode_T* arg = struct_lit->args->items[i];
        if(arg->data_type)
        {
            ASTNode_T* member = init_ast_node(&v->context->raw_allocator, ND_STRUCT_MEMBER, arg->tok);
            char buffer[100] = {0};
            sprintf(buffer, "_%ld", i);
            char* buf_ptr = strdup(buffer);
            CONTEXT_ALLOC_REGISTER(v->context, (void*) buf_ptr);
            member->id = init_ast_identifier(&v->context->raw_allocator, arg->tok, buf_ptr);
            member->data_type = arg->data_type;

            list_push(type->members, member);
        }
        else
            throw_error(v->context, ERR_TYPE_ERROR, arg->tok, "cannot resolve data type");
    }
    struct_lit->data_type = type;
}

static void iter_validate_struct_lit(ASTNode_T* struct_lit, va_list args)
{
    GET_VALIDATOR(args);

    if(!struct_lit->data_type)
        validate_anon_struct_lit(v, struct_lit);
}

static void iter_validate_array_lit(ASTNode_T* array_lit, va_list args)
{
    GET_VALIDATOR(args);

    array_lit->data_type = init_ast_type(&v->context->raw_allocator, TY_ARRAY, array_lit->tok);

    if(array_lit->args->size == 0)
        throw_error(v->context, ERR_UNDEFINED, array_lit->tok, "empty array literals are not allowed");
    
    ASTNode_T* first_arg = array_lit->args->items[0];
    if(first_arg->unpack_mode)
    {
        ASTType_T* base_type = unpack(first_arg->data_type)->base;
        if(!base_type) {
            throw_error(v->context, ERR_TYPE_ERROR_UNCR, first_arg->tok, "unpacking with `...` is only supported for array types\n");
        }
        array_lit->data_type->base = base_type;
    }
    else
        array_lit->data_type->base = first_arg->data_type;
    array_lit->data_type->num_indices = 0;

    for(size_t i = 0; i < array_lit->args->size; i++)
    {
        ASTNode_T* arg = array_lit->args->items[i];
        if(!arg->unpack_mode)
        {
            array_lit->data_type->num_indices++;
            continue;
        }

        array_lit->data_type->num_indices += unpack(first_arg->data_type)->num_indices;
    }

    array_lit->data_type->size = get_type_size(v, array_lit->data_type);
}

static void iter_validate_ternary(ASTNode_T* ternary, va_list args)
{
    GET_VALIDATOR(args);

    if(!is_bool(ternary->condition->data_type))
        throw_error(v->context, ERR_TYPE_ERROR_UNCR, ternary->condition->tok, "expect `bool` type for if condition");
    
    ternary->data_type = ternary->if_branch->data_type;

    if(!types_equal(v->context, ternary->if_branch->data_type, ternary->else_branch->data_type))
        throw_error(v->context, ERR_TYPE_ERROR_UNCR, ternary->tok, "data types for ternary branches do not match");
}

static void iter_validate_else_expr(ASTNode_T* else_expr, va_list args)
{
    GET_VALIDATOR(args);
    else_expr->data_type = else_expr->left->data_type;

    if(!types_equal(v->context, else_expr->left->data_type, else_expr->right->data_type))
        throw_error(v->context, ERR_TYPE_ERROR_UNCR, else_expr->tok, "data types of `else` branches do not match");
}

static void iter_validate_len(ASTNode_T* len, va_list args)
{
    GET_VALIDATOR(args);

    ASTType_T* ty = unpack(len->expr->data_type);
    if(ty->kind != TY_C_ARRAY && ty->kind != TY_ARRAY && ty->kind != TY_VLA && !(ty->kind == TY_PTR && unpack(ty->base)->kind == TY_CHAR))
    {
        char* buf = malloc(BUFSIZ * sizeof(char));
        *buf = '\0';
        throw_error(v->context, ERR_TYPE_ERROR, len->tok, "cannot get length of type `%s`, expect array or `&char`",
            ast_type_to_str(v->context, buf, len->expr->data_type, BUFSIZ));
        free(buf);
    }
}

static void iter_enter_pipe(ASTNode_T* pipe, va_list args)
{
    GET_VALIDATOR(args);

    pipe->expr = v->current_pipe;
    v->current_pipe = pipe;
}

static void iter_leave_pipe(ASTNode_T* pipe, va_list args)
{
    GET_VALIDATOR(args);
    pipe->data_type = pipe->right->data_type;

    if(pipe->right->kind == ND_HOLE)
    {
        throw_error(v->context, ERR_SYNTAX_WARNING, pipe->tok, "unnecessary `|>` expression");
        *pipe = *pipe->left;
    }

    v->current_pipe = pipe->expr;
}

static void iter_validate_hole(ASTNode_T* hole, va_list args)
{
    GET_VALIDATOR(args);
    if(!v->current_pipe)
        throw_error(v->context, ERR_SYNTAX_ERROR, hole->tok, "hole expression not in pipe");
    
    if(!v->current_pipe->left->data_type)
        throw_error(v->context, ERR_TYPE_ERROR_UNCR, hole->tok, "cannot resolve data type of pipe input expression");
    hole->data_type = v->current_pipe->left->data_type;
    hole->referenced_obj = v->current_pipe->left->referenced_obj;
    hole->expr = v->current_pipe;
}

static void iter_enter_lambda(ASTNode_T* lambda, va_list args)
{
    static u64 id = 0;
    lambda->long_val = id++;
}

static void iter_leave_lambda(ASTNode_T* lambda, va_list args)
{
    GET_VALIDATOR(args);

/*    if(v->context->ct != CT_ASM)
        return;
    ASTObj_T* lambda_stack_ptr = init_ast_obj(&v->context->raw_allocator, OBJ_GLOBAL, lambda->tok);
    lambda_stack_ptr->data_type = (ASTType_T*) void_ptr_type;

    char* id = calloc(64, sizeof(char));
    CONTEXT_ALLOC_REGISTER(v->context, (void*) id);
    sprintf(id, "lambda.stackptr.%ld", lambda->long_val);

    lambda_stack_ptr->id = init_ast_identifier(&v->context->raw_allocator, lambda->tok, id);

    list_push(v->ast->objs, lambda_stack_ptr);
    lambda->stack_ptr = lambda_stack_ptr;

    ASTType_T* return_type = unpack(lambda->data_type->base);
    if(return_type->kind == TY_C_ARRAY) 
        throw_error(v->context, ERR_TYPE_ERROR_UNCR, return_type->tok ? return_type->tok : lambda->tok, "cannot return an array type from a function");
    else if(return_type->kind == TY_STRUCT && return_type->size > 16)
    {
        lambda->return_ptr = init_ast_obj(&v->context->raw_allocator, OBJ_LOCAL, lambda->data_type->base->tok);
        lambda->return_ptr->data_type = init_ast_type(&v->context->raw_allocator, TY_PTR, lambda->data_type->base->tok);
        lambda->return_ptr->data_type->base = lambda->data_type->base;
        lambda->return_ptr->data_type->size = get_type_size(v, lambda->return_ptr->data_type);
        lambda->return_ptr->data_type->align = 8;
    } */
    // FIXME
}

static void iter_validate_string_lit(ASTNode_T* str, va_list args)
{
    GET_VALIDATOR(args);

       // check for invalid escape sequences
    for(size_t i = 0; i < strlen(str->str_val); i++)
    {
        if(str->str_val[i] == '\\')
        {
            switch(str->str_val[++i])
            {
                case 'a':
                case 'b':
                case 't':
                case 'v':
                case 'n':
                case 'r':
                case 'f':
                case 'e':
                case '"':
                case '\'':
                case '\\':  
                case '0':
                    continue;
                
                case 'x':
                    if(!isxdigit(str->str_val[++i]) || !isxdigit(str->str_val[++i]))
                        throw_error(v->context, ERR_SYNTAX_ERROR_UNCR, str->tok, "invalid escape sequence `\\x%c%c`, chars after `\\x` must be hexadecimal digits", str->str_val[i - 1], str->str_val[i]);
                    continue;

                default:
                    throw_error(v->context, ERR_SYNTAX_ERROR_UNCR, str->tok, "invalid escape sequence `\\%c` found in string literal", str->str_val[i]);
                    return;
            }
        }
    }
}

static void iter_validate_char_lit(ASTNode_T* ch, va_list args)
{
     GET_VALIDATOR(args);

    // check for invalid escape sequences and evaluate them
    char c = ch->str_val[0];

    if(c == '\\')
    {
        switch(ch->str_val[1])
        {
            case 'a':
                c = '\a';
                break;
            case 'b':
                c = '\b';
                break;
            case 't':
                c = '\t';
                break;
            case 'v':
                c = '\v';
                break;
            case 'n':
                c = '\n';
                break;
            case 'r':
                c = '\r';
                break;
            case 'f':
                c = '\f';
                break;
            case '\'':
                c = '\'';
                break;
            case '"':
                c = '"';
                break;
            case '\\':
                c = '\\';
                break;  
            case '0':
                c = '\0';
                break;
            default:
                throw_error(v->context, ERR_SYNTAX_ERROR_UNCR, ch->tok, "invalid escape sequence `\\%c` found in char literal", ch->str_val[1]);
                return;
        }
    }
    
    free(ch->str_val);
    ch->int_val = c;
}

static void iter_validate_type_expr(ASTNode_T* cmp, va_list args)
{
     GET_VALIDATOR(args);

    // type comparisons have to be evaluated at compile-time
    bool result = false;

    switch(cmp->cmp_kind)
    {
        case ND_EQ:
            result = types_equal(v->context, cmp->l_type, cmp->r_type);
            break;
        case ND_NE:
            result = !types_equal(v->context, cmp->l_type, cmp->r_type);
            break;
        case ND_GT:
            result = cmp->l_type->size > cmp->r_type->size;
            break;
        case ND_GE:
            result = cmp->l_type->size >= cmp->r_type->size;
            break;
        case ND_LT:
            result = cmp->l_type->size < cmp->r_type->size;
            break;
        case ND_LE:
            result = cmp->l_type->size <= cmp->r_type->size;
            break;
        case ND_BUILTIN_REG_CLASS:
            {
                cmp->kind = ND_INT;
                cmp->int_val = 2;

                ASTType_T* expanded = unpack(cmp->r_type);
                if(is_integer(expanded) || is_ptr(expanded))
                    cmp->int_val = 0;
                else if(is_flonum(expanded))
                    cmp->int_val = 1;
            } return;
        case ND_BUILTIN_IS_INT...ND_BUILTIN_IS_UNION:
            {
                cmp->kind = ND_BOOL;
                ASTType_T* expanded = unpack(cmp->r_type);
                switch(cmp->cmp_kind)
                {
                    case ND_BUILTIN_IS_INT:
                        cmp->bool_val = is_integer(expanded) && !is_unsigned(expanded);
                        break;
                    case ND_BUILTIN_IS_UINT:
                        cmp->bool_val = is_integer(expanded) && is_unsigned(expanded);
                        break;
                    case ND_BUILTIN_IS_FLOAT:
                        cmp->bool_val = is_flonum(expanded);
                        break;
                    case ND_BUILTIN_IS_POINTER:
                        cmp->bool_val = expanded->kind == TY_PTR;
                        break;
                    case ND_BUILTIN_IS_ARRAY:
                        cmp->bool_val = expanded->kind == TY_C_ARRAY || expanded->kind == TY_ARRAY || expanded->kind == TY_VLA;
                        break;
                    case ND_BUILTIN_IS_STRUCT:
                        cmp->bool_val = expanded->kind == TY_STRUCT && !expanded->is_union;
                        break;
                    case ND_BUILTIN_IS_UNION:
                        cmp->bool_val = expanded->kind == TY_STRUCT && expanded->is_union;
                        break;
                    default:
                        unreachable();
                }
            } return;
        case ND_BUILTIN_TO_STR:
            {
                char* buf = malloc(BUFSIZ * sizeof(char));
                *buf = '\0';
                ASTNode_T node = {
                    .kind = ND_STR,
                    .tok = cmp->tok,
                    .str_val = strdup(ast_type_to_str(v->context, buf, cmp->r_type, BUFSIZ)),
                    .data_type = (ASTType_T*) char_ptr_type
                };
                *cmp = node;
                CONTEXT_ALLOC_REGISTER(v->context, (void*) buf);
            } return;
        default: 
            unreachable();
    }   

    // convert the expression to a constant value
    cmp->kind = ND_BOOL;
    cmp->bool_val = result;
    cmp->data_type = (ASTType_T*) primitives[TY_BOOL];
}

static void iter_validate_struct_type(ASTType_T* struct_type, va_list args);
static void iter_validate_enum_type(ASTType_T* struct_type, va_list args);
static void iter_validate_typeof_type(ASTType_T* struct_type, va_list args);
static void iter_validate_array_type(ASTType_T* struct_type, va_list args);
static void iter_validate_c_array_type(ASTType_T* struct_type, va_list args);
static void iter_validate_type(ASTType_T* type, va_list args);

static void validate_obj_deep(Validator_T* v, ASTObj_T* obj)
{
    static const ASTIteratorList_T iter = {
        .obj_start_fns = {
            [OBJ_FUNCTION] = iter_enter_function,
            [OBJ_TYPEDEF] = iter_enter_typedef,
            [OBJ_FN_ARG] = iter_enter_fn_arg,
            [OBJ_ENUM_MEMBER] = iter_enter_enum_member,
            [OBJ_GLOBAL] = iter_enter_global,
        },
        .obj_end_fns = {
            [OBJ_FUNCTION] = iter_leave_function,
            [OBJ_TYPEDEF] = iter_leave_typedef,
            [OBJ_FN_ARG] = iter_leave_fn_arg,
            [OBJ_ENUM_MEMBER] = iter_leave_enum_member,
            [OBJ_GLOBAL] = iter_leave_global,
        },
        .node_start_fns = {
            [ND_BLOCK] = iter_enter_block,
            [ND_ASSIGN] = iter_enter_assignment,
            [ND_PIPE] = iter_enter_pipe,
            [ND_LAMBDA] = iter_enter_lambda,
        },
        .node_end_fns = {
            [ND_BLOCK] = iter_leave_block,
            [ND_RETURN] = iter_validate_return,
            [ND_FOR_RANGE] = iter_validate_for_range,
            [ND_MATCH_TYPE] = iter_validate_match_type,
            [ND_WITH] = iter_validate_with,
            [ND_CASE] = iter_validate_case,
            [ND_EXPR_STMT] = iter_validate_expr_stmt,
            [ND_EXTERN_C_BLOCK] = iter_validate_extern_c_block,
            [ND_CALL] = iter_validate_call,
            [ND_ID] = iter_validate_identifier,
            [ND_CLOSURE] = iter_validate_closure,
            [ND_REF] = iter_validate_reference,
            [ND_DEREF] = iter_validate_dereference,
            [ND_MEMBER] = iter_validate_member,
            [ND_ADD] = iter_validate_binop,
            [ND_SUB] = iter_validate_binop,
            [ND_MUL] = iter_validate_binop,
            [ND_DIV] = iter_validate_binop,
            [ND_MOD] = iter_validate_modulo,
            [ND_NEG] = iter_validate_negation,
            [ND_BIT_NEG] = iter_validate_bitwise_negation,
            [ND_NOT] = iter_validate_not,
            [ND_EQ] = iter_validate_equals,
            [ND_NE] = iter_validate_equals,
            [ND_LT] = iter_validate_lt_gt,
            [ND_LE] = iter_validate_lt_gt,
            [ND_GT] = iter_validate_lt_gt,
            [ND_GE] = iter_validate_lt_gt,
            [ND_AND] = iter_validate_and_or,
            [ND_OR] = iter_validate_and_or,
            [ND_XOR] = iter_validate_bitwise_op,
            [ND_LSHIFT] = iter_validate_bitwise_op,
            [ND_RSHIFT] = iter_validate_bitwise_op,
            [ND_BIT_OR] = iter_validate_bitwise_op,
            [ND_BIT_AND] = iter_validate_bitwise_op,
            [ND_INC] = iter_validate_inc_dec,
            [ND_DEC] = iter_validate_inc_dec,
            [ND_INDEX] = iter_validate_index,
            [ND_ASSIGN] = iter_leave_assignment,
            [ND_STRUCT] = iter_validate_struct_lit,
            [ND_ARRAY] = iter_validate_array_lit,
            [ND_TERNARY] = iter_validate_ternary,
            [ND_ELSE_EXPR] = iter_validate_else_expr,
            [ND_LEN] = iter_validate_len,
            [ND_PIPE] = iter_leave_pipe,
            [ND_HOLE] = iter_validate_hole,
            [ND_LAMBDA] = iter_leave_lambda,
            [ND_STR] = iter_validate_string_lit,
            [ND_CHAR] = iter_validate_char_lit,
            [ND_TYPE_EXPR] = iter_validate_type_expr,
        },
        .type_fns = {
            [TY_STRUCT] = iter_validate_struct_type,
            [TY_ENUM] = iter_validate_enum_type,
            [TY_TYPEOF] = iter_validate_typeof_type,
            [TY_ARRAY] = iter_validate_array_type,
            [TY_C_ARRAY] = iter_validate_c_array_type,
        },
        .type_end = iter_validate_type,
    };

    ast_iterate_obj(&iter, obj, v);
}

static void validate_semantics(Validator_T* v, ResolveQueue_T* queue)
{
    ResolveQueueNode_T* node = queue->head;
    while(node)
    {
        validate_obj_shallow(v, node->obj);

        if(node->method & RESOLVE_DEEP)
            validate_obj_deep(v, node->obj);

        node = node->next;
    }
}

static bool struct_contains_embeds(ASTType_T* struct_type)
{
    for(size_t i = 0; i < struct_type->members->size; i++)
    {
        ASTNode_T* member = struct_type->members->items[i];
        if(member->kind == ND_EMBED_STRUCT)
            return true;
    }
    return false;
}

static void resolve_struct_embeds(Validator_T* v, ASTType_T* struct_type)
{
    if(!struct_contains_embeds(struct_type))
        return;

    List_T* resolved_fields = init_list_sized(struct_type->members->size);
    CONTEXT_ALLOC_REGISTER(v->context, resolved_fields);

    for(size_t i = 0; i < struct_type->members->size; i++)
    {
        ASTNode_T* member = struct_type->members->items[i];
        switch(member->kind)
        {
            case ND_STRUCT_MEMBER:
                list_push(resolved_fields, member);
                break;
            
            case ND_EMBED_STRUCT:
            {
                char buf[BUFSIZ] = {'\0'};
                
                ASTType_T* embedded_type = unpack(member->data_type);
                if(embedded_type->kind != TY_STRUCT)
                {
                    throw_error(v->context, ERR_TYPE_ERROR, member->data_type->tok, "embedding types is only supported for structs, got `%s`", ast_type_to_str(v->context, buf, embedded_type, LEN(buf)));
                    break;
                }

                resolve_struct_embeds(v, embedded_type);
                for(size_t i = 0; i < embedded_type->members->size; i++)
                    list_push(resolved_fields, embedded_type->members->items[i]);
            } break;
            default:
                unreachable();
        }
    }
    struct_type->members = resolved_fields;
}

static void iter_validate_struct_type(ASTType_T* struct_type, va_list args)
{
    GET_VALIDATOR(args);
    resolve_struct_embeds(v, struct_type);

    for(size_t i = 0; i < struct_type->members->size; i++)
    {
        ASTNode_T* member = struct_type->members->items[i];
        ASTType_T* expanded = unpack(member->data_type);

        switch(member->kind)
        {
            case ND_STRUCT_MEMBER:
                if(expanded->kind == TY_VOID)
                    throw_error(v->context, ERR_TYPE_ERROR_UNCR, member->data_type->tok, "struct member cannot be of type `void`");

                break;
            case ND_EMBED_STRUCT:
                unreachable();
                break;
            default:
                unreachable();
        }
    }
}

static void iter_validate_typeof_type(ASTType_T* typeof_type, va_list args)
{
    GET_VALIDATOR(args);

    ASTType_T* found = typeof_type->num_indices_node->data_type;
    if(!found)
        throw_error(v->context, ERR_TYPE_ERROR, typeof_type->num_indices_node->tok, "could not resolve data type");
    
    *typeof_type = *found;
    typeof_type->no_warnings = true;
}

static void iter_validate_array_type(ASTType_T* array_type, va_list args)
{
    GET_VALIDATOR(args);
    if(array_type->num_indices_node)
        array_type->num_indices = const_u64(v->context, eval_constexpr(&v->constexpr_resolver, array_type->num_indices_node));
}

static void iter_validate_c_array_type(ASTType_T* ca_type, va_list args)
{
    GET_VALIDATOR(args);
    if(ca_type->num_indices_node)
        ca_type->num_indices = const_u64(v->context, ca_type->num_indices_node);
}

static void iter_validate_enum_type(ASTType_T* enum_type, va_list args)
{
    GET_VALIDATOR(args);

    for(size_t i = 0; i < enum_type->members->size; i++)
    {
        ASTObj_T* member = enum_type->members->items[i];
        if(member->value->kind != ND_NOOP)
        {
            member->value->int_val = (i32) const_i64(v->context, eval_constexpr(&v->constexpr_resolver, member->value));
            member->value->kind = ND_INT;
        }
        else 
        {
            member->value->int_val = i ? ((ASTObj_T*) enum_type->members->items[i - 1])->value->int_val + 1 : 0;
            member->value->kind = ND_INT;
        }
    }
}

static i32 align_type(ASTType_T* ty)
{
    switch(ty->kind)
    {
        case TY_C_ARRAY:
        case TY_PTR:
            return MAX(pow(2, floor(log(ty->base->size)/log(2))), 8);
        default:
            return MAX(pow(2, floor(log(ty->size)/log(2))), 1);
    }
}

static void iter_validate_type(ASTType_T* type, va_list args)
{
    GET_VALIDATOR(args);
    ASTType_T* exp = unpack(type);

    type->size = get_type_size(v, exp);
    type->align = align_type(exp);
    exp->align = type->align;
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

static i32 get_struct_size(Validator_T* v, ASTType_T* struct_type, List_T** struct_types)
{
    if(!*struct_types)
        *struct_types = init_list();

    if(list_contains(*struct_types, struct_type)) {
        char buf[BUFSIZ] = {};
        throw_error(v->context, ERR_TYPE_ERROR, struct_type->tok, "detected recursive structs, encountered `%s` twice.", ast_type_to_str(v->context, buf, struct_type, LEN(buf)));
        return 0;
    }

    list_push(*struct_types, struct_type);

    i64 bits = 0;
    for(size_t i = 0; i < struct_type->members->size; i++)
    {
        ASTNode_T* member = struct_type->members->items[i];
        member->data_type->size = get_type_size_impl(v, member->data_type, struct_types);
        bits = align_to(bits, get_type_align(v, member->data_type) * 8);
        member->offset = bits / 8;
        bits += member->data_type->size * 8;
    }

    list_pop(*struct_types);

    return align_to(bits, get_type_align(v, struct_type) * 8) / 8;
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
            return get_type_size_impl(v, unpack(type->num_indices_node->data_type), struct_types);
        case TY_UNDEF:
            return get_type_size_impl(v, unpack(type), struct_types);
        case TY_C_ARRAY:
            if(type->num_indices == 0)
                return 0;
            return get_type_size_impl(v, type->base, struct_types) * calculate_array_size(v, type);
        case TY_VLA:
            return PTR_S;
        case TY_ARRAY:
                return get_type_size_impl(v, type->base, struct_types) * calculate_array_size(v, type) + PTR_S;
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

