#ifndef CSPYDR_PARSER_QUEUE_H
#define CSPYDR_PARSER_QUEUE_H

#include "ast/ast.h"
#include "validator.h"

typedef enum RESOLVE_METHOD
{
    RESOLVE_DEEP    = 0b01,
    RESOLVE_SHALLOW = 0b10,
    RESOLVE_ANYHOW  = 0b11
} ResolveMethod_T;

typedef struct RESOLVE_QUEUE_NODE_STRUCT
{
    struct RESOLVE_QUEUE_NODE_STRUCT* next;
    ResolveMethod_T method;
    ASTObj_T* obj;
} ResolveQueueNode_T;

ResolveQueueNode_T* resolve_queue_node_init(ResolveMethod_T method, ASTObj_T* obj);
void resolve_queue_node_free(ResolveQueueNode_T* node);

typedef struct RESOLVE_QUEUE_STRUCT
{
    ResolveQueueNode_T* head;
} ResolveQueue_T;

void resolve_queue_init(ResolveQueue_T* queue);
void resolve_queue_free(ResolveQueue_T* queue);

ResolveQueueNode_T* resolve_queue_last(ResolveQueue_T* queue);
bool resolve_queue_contains(ResolveQueue_T* queue, ASTObj_T* obj, ResolveMethod_T method);

void build_resolve_queue(Validator_T* v, ResolveQueue_T* queue);

#endif

