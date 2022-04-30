#include "utils.h"

#include "ast/ast_iterator.h"
#include "ast/types.h"
#include "list.h"
#include "globals.h"
#include "mem/mem.h"
#include "toolchain.h"

#include <stdarg.h>
#include <string.h>

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

void collect_locals_array_lit(ASTNode_T* a_lit, va_list args)
{
    GET_LIST(args);
    if(global.ct == CT_ASM)
        list_push(l, a_lit->buffer);
}

void collect_locals(ASTNode_T* stmt, List_T* locals)
{
    static ASTIteratorList_T iterator = {
        .node_start_fns = {
            [ND_BLOCK] = collect_locals_block,
            [ND_WITH] = collect_locals_with,
            [ND_FOR] = collect_locals_for,
            [ND_LAMBDA] = collect_locals_lambda,
            [ND_ARRAY] = collect_locals_array_lit,
        }
    };

    ast_iterate_stmt(&iterator, stmt, locals);
}

#undef GET_LIST

bool identifiers_equal(ASTIdentifier_T* a, ASTIdentifier_T* b)
{
    if(!a->outer != !b->outer || strcmp(a->callee, b->callee) != 0)
        return false;
    else if(a->outer)
        return identifiers_equal(a->outer, b->outer);
    else
        return true;
}

// function to quickly build string literals
ASTNode_T* build_str_lit(Token_T* tok, char* str, bool allocate_global, List_T* objs)
{
    ASTNode_T* str_lit = init_ast_node(ND_STR, tok);
    str_lit->is_constant = true;
    str_lit->data_type = (ASTType_T*) char_ptr_type;
    str_lit->str_val = str;
    mem_add_ptr(str);

    if(global.ct == CT_ASM && allocate_global)
    {
        static u64 i = 0;
        ASTIdentifier_T* ast_id = init_ast_identifier(str_lit->tok, (char[]){'\0'});
        sprintf(ast_id->callee, "L.str.%ld", i++);

        ASTObj_T* globl = init_ast_obj(OBJ_GLOBAL, str_lit->tok);
        globl->id = ast_id;
        globl->value = str_lit;
        globl->data_type = init_ast_type(TY_C_ARRAY, str_lit->tok);
        globl->data_type->num_indices = strlen(str_lit->str_val) + 1;
        globl->data_type->base = (ASTType_T*) primitives[TY_CHAR];
        list_push(objs, globl);

        ASTNode_T* caller = init_ast_node(ND_ID, str_lit->tok);
        caller->id = ast_id;
        caller->referenced_obj = globl;
        caller->data_type = globl->data_type;//(ASTType_T*) char_ptr_type;

        return caller;
    }
    else
        return str_lit;
}