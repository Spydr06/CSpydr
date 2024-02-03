#include "queue.h"
#include "ast/ast.h"
#include "ast/ast_iterator.h"
#include "error/error.h"
#include "list.h"
#include "parser/validator.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#define GET_ARGS(va)                                    \
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

static void resolve_obj_enqueue(Validator_T* v, ResolveQueue_T* queue, ASTObj_T* obj, ResolveMethod_T method)
{
    validator_push_obj(v, obj);
    if(resolve_queue_contains(queue, obj, method == RESOLVE_SHALLOW ? RESOLVE_ANYHOW : method))
        goto finish;

    if(list_contains(v->obj_stack, obj))
       goto finish;

    static const ASTIteratorList_T iter = {
        .node_end_fns = {
            [ND_ID] = resolve_id_enqueue
        },
        .type_fns = {
            [TY_UNDEF] = resolve_type_alias_enqueue
        }
    };

    ast_iterate_obj(&iter, obj, v, queue);
    if(obj->kind == OBJ_LOCAL && obj->value)
        ast_iterate_expr(&iter, obj->value, v, queue);

    if(resolve_queue_contains(queue, obj, method == RESOLVE_SHALLOW ? RESOLVE_ANYHOW : method))
        goto finish;

    enqueue_obj(queue, obj, method == RESOLVE_ANYHOW ? RESOLVE_SHALLOW : RESOLVE_DEEP);

finish:
    validator_pop_obj(v);
}

static void resolve_id_enqueue(ASTNode_T* ident, va_list args)
{
    GET_ARGS(args);
    assert(ident->referenced_obj != NULL);
    resolve_obj_enqueue(v, queue, ident->referenced_obj, RESOLVE_SHALLOW);
}

static void resolve_type_alias_enqueue(ASTType_T* alias, va_list args)
{
    GET_ARGS(args);
    assert(alias->referenced_obj != NULL);
    resolve_obj_enqueue(v, queue, alias->referenced_obj, RESOLVE_SHALLOW);
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
            [OBJ_LOCAL] = init_obj_enqueue,
            [OBJ_FUNCTION] = init_obj_enqueue,
            [OBJ_FN_ARG] = init_obj_enqueue,
            [OBJ_ENUM_MEMBER] = init_obj_enqueue,
            [OBJ_TYPEDEF] = init_obj_enqueue
        },
    };

    ast_iterate(&iter, v->ast, v, queue);
}

