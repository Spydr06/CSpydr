#include "utils.h"

#include "ast/ast_iterator.h"
#include "ast/types.h"
#include "list.h"
#include <stdarg.h>

ASTObj_T alloca_bottom = {
    .kind = OBJ_LOCAL,
    .id = &(ASTIdentifier_T) {
        .callee = "__alloca_size__"
    },
    .data_type = &(ASTType_T){
        .size = PTR_S,
        .align = PTR_S
    },
    .offset = 0
};

#define GET_LIST(va) List_T* l = va_arg(va, List_T*)

void collect_locals_block(ASTNode_T* block, va_list args)
{
    GET_LIST(args);
    for(size_t i = 0; i < block->locals->size; i++)
        list_push(l, block->locals->items[i]);
}

void collect_locals_with(ASTNode_T* with, va_list args)
{
    GET_LIST(args);
    list_push(l, with->obj);
}

void collect_locals_for(ASTNode_T* for_stmt, va_list args)
{
    GET_LIST(args);
    for(size_t i = 0; i < for_stmt->locals->size; i++)
        list_push(l, for_stmt->locals->items[i]);
}

void collect_locals_lambda(ASTNode_T* lambda, va_list args)
{
    GET_LIST(args);
    for(size_t i = 0; i < lambda->args->size; i++)
        list_push(l, lambda->args->items[i]);
}

void collect_locals(ASTNode_T* stmt, List_T* locals)
{
    static ASTIteratorList_T iterator = {
        .node_start_fns = {
            [ND_BLOCK] = collect_locals_block,
            [ND_WITH] = collect_locals_with,
            [ND_FOR] = collect_locals_for,
            [ND_LAMBDA] = collect_locals_lambda
        }
    };

    ast_iterate_stmt(&iterator, stmt, locals);
}

#undef GET_LIST
