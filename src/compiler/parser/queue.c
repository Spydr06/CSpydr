#include "queue.h"
#include "ast/ast.h"
#include "ast/ast_iterator.h"
#include "error/error.h"
#include "list.h"
#include "parser/validator.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

#define GET_ARGS(va)                                                            \
    Validator_T* v = va_arg(va, Validator_T*);          \
    ResolveQueue_T* queue = va_arg(va, ResolveQueue_T*)

ResolveQueueNode_T* resolve_queue_node_init(ResolveMethod_T method, ASTObj_T* obj)
{
    ResolveQueueNode_T* node = calloc(1, sizeof(struct RESOLVE_QUEUE_NODE_STRUCT));
    node->method = method;
    node->obj = obj;
    return node;
}

void resolve_queue_node_free(ResolveQueueNode_T* node)
{
    if(!node)
        return;

    resolve_queue_node_free(node->next);
    free(node);
}

void resolve_queue_init(ResolveQueue_T* queue)
{
    queue->head = NULL;    
}

void resolve_queue_free(ResolveQueue_T* queue)
{
    resolve_queue_node_free(queue->head);
}

ResolveQueueNode_T* resolve_queue_last(ResolveQueue_T* queue)
{
    ResolveQueueNode_T* node = queue->head;
    while(node && node->next)
        node = node->next;
    return node;
}

bool resolve_queue_contains(ResolveQueue_T* queue, ASTObj_T* obj, ResolveMethod_T method)
{
    ResolveQueueNode_T* node = queue->head;
    while(node)
    {
        if(node->obj == obj && node->method & method)
            return true;
        node = node->next;
    }
    return false;
}

static void enqueue_obj(ResolveQueue_T* queue, ASTObj_T* obj, ResolveMethod_T method)
{
    ResolveQueueNode_T* new = resolve_queue_node_init(method, obj);
    ResolveQueueNode_T* tail = resolve_queue_last(queue);
    if(!tail)
        queue->head = new;
    else
        tail->next = new;
}

static void resolve_id_enqueue(ASTNode_T* ident, va_list args);
static void resolve_type_alias_enqueue(ASTType_T* alias, va_list args);

static void report_recursive_dependency(Validator_T* v, ASTObj_T* obj)
{
    size_t index = list_contains(v->obj_stack, obj) - 1;

    size_t stack_size = v->obj_stack->size - index;
    char* stack_buf = calloc(stack_size, sizeof(char) * BUFSIZ);
    
    char* obj_buf = ast_id_to_str(calloc(BUFSIZ, sizeof(char)), obj->id, BUFSIZ);

    if(!stack_size)
    {
        throw_error(v->context, ERR_CIRC_DEP, obj->tok, "%s `%s` depends on itself", obj_kind_to_str(obj->kind), obj_buf);
        goto finish;
    }

    char* cursor = stack_buf;
    for(size_t i = 0; i < stack_size; i++)
    {
        ASTObj_T* obj = v->obj_stack->items[v->obj_stack->size - 1 - i];
        snprintf(cursor, BUFSIZ, COLOR_RESET "  %s:%d:%d: " COLOR_BOLD_WHITE, EITHER(obj->tok->source->short_path, obj->tok->source->path), obj->tok->line + 1, obj->tok->pos);

        ast_id_to_str(cursor, obj->id, BUFSIZ - strlen(cursor) - 1);

        if(obj->kind == OBJ_FUNCTION)
            strcat(cursor, "()");

        if(stack_size - i > 1)
            strcat(cursor, "\n");
        cursor += strlen(cursor);
    }

    throw_error(
        v->context,
        ERR_CIRC_DEP,
        obj->tok,
        "%s `%s` depends on itself via:\n%s" COLOR_RESET,
        obj_kind_to_str(obj->kind),
        obj_buf,
        stack_buf
    );

finish:
    free(obj_buf);
    free(stack_buf);
}

static void enter_type(ASTType_T* type, va_list args)
{
    GET_ARGS(args);
    if(type->kind == TY_VLA || type->kind == TY_PTR)
        validator_push_type_info(v, false);        
}

static void leave_type(ASTType_T* type, va_list args)
{
    GET_ARGS(args);
    if(type->kind == TY_VLA || type->kind == TY_PTR)
        validator_pop_type_info(v);
}     


static void resolve_obj_enqueue(Validator_T* v, ResolveQueue_T* queue, ASTObj_T* obj, ResolveMethod_T method)
{
    assert(method != RESOLVE_ANYHOW);

    validator_push_obj(v, obj);
    if(resolve_queue_contains(queue, obj, method == RESOLVE_SHALLOW ? RESOLVE_ANYHOW : method))
        goto finish;

    static ASTIteratorList_T iter = {
        .node_end_fns = {
            [ND_ID] = resolve_id_enqueue
        },
        .type_begin = enter_type,
        .type_end = leave_type,
        .type_fns = {
            [TY_UNDEF] = resolve_type_alias_enqueue
        }
    };

    if(method == RESOLVE_DEEP)
    {
        if(list_contains(v->obj_stack, obj))
        {
            report_recursive_dependency(v, obj);
            goto finish;
        }

        iter.iterate_only_objs = false;

    }
    else
    {
        iter.iterate_only_objs = obj->kind == OBJ_FUNCTION;
    }

    ast_iterate_obj(&iter, obj, v, queue);

    if(resolve_queue_contains(queue, obj, method == RESOLVE_SHALLOW ? RESOLVE_ANYHOW : method))
            goto finish;

    enqueue_obj(queue, obj, method);

finish:
    validator_pop_obj(v);
}

static void resolve_id_enqueue(ASTNode_T* ident, va_list args)
{
    GET_ARGS(args);
    assert(ident->referenced_obj != NULL);
    switch(ident->referenced_obj->kind)
    {
    case OBJ_GLOBAL:
        resolve_obj_enqueue(v, queue, ident->referenced_obj, ident->referenced_obj->constexpr ? RESOLVE_DEEP : RESOLVE_SHALLOW);
        break;
    case OBJ_ENUM_MEMBER:
        resolve_obj_enqueue(v, queue, ident->referenced_obj, RESOLVE_DEEP);
        break;
    case OBJ_FUNCTION:
        {
            ResolveMethod_T method = 
                (v->current_obj->constexpr || v->current_obj->kind == OBJ_TYPEDEF) && v->current_obj != ident->referenced_obj 
                ? RESOLVE_DEEP : RESOLVE_SHALLOW;
            resolve_obj_enqueue(v, queue, ident->referenced_obj, method);
        } break;
    default:
        break;
    }
}

static void resolve_type_alias_enqueue(ASTType_T* alias, va_list args)
{
    GET_ARGS(args);
    assert(alias->referenced_obj != NULL);

    if(v->need_exact_type_infos)
        resolve_obj_enqueue(v, queue, alias->referenced_obj, RESOLVE_DEEP);
}

static void init_obj_enqueue(ASTObj_T* obj, va_list args)
{
    GET_ARGS(args);
    resolve_obj_enqueue(v, queue, obj, RESOLVE_DEEP);
}

void build_resolve_queue(Validator_T* v, ResolveQueue_T* queue)
{
    static const ASTIteratorList_T iter = {
        .obj_end_fns = {
            [OBJ_GLOBAL] = init_obj_enqueue,
            [OBJ_FUNCTION] = init_obj_enqueue,
            [OBJ_ENUM_MEMBER] = init_obj_enqueue,
            [OBJ_TYPEDEF] = init_obj_enqueue
        },
        .type_begin = enter_type,
        .type_end = leave_type,
        .type_fns = {
            [TY_UNDEF] = resolve_type_alias_enqueue
        },
    };

    v->need_exact_type_infos = true;
    ast_iterate(&iter, v->ast, v, queue);
    assert(v->exact_type_info_stack->size == 0);
}

void dbg_print_resolve_queue(ResolveQueue_T *queue)
{
    static const char* METHOD_STRINGS[] = {
        [RESOLVE_DEEP] = "deep",
        [RESOLVE_SHALLOW] = "shallow",
        [RESOLVE_ANYHOW] = "anyhow"
    };

    ResolveQueueNode_T* node = queue->head;
    char* buf = malloc(BUFSIZ * sizeof(char));
    printf("Queued objects:\n");
    for(size_t i = 1; node; i++) {
        *buf = '\0';
        ast_id_to_str(buf, node->obj->id, BUFSIZ);
        printf("[%8zu] %s (%s)\n", i, buf, METHOD_STRINGS[node->method]);
        node = node->next;
    }
}

