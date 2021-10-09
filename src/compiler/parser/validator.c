#include "validator.h"
#include "../ast/ast_iterator.h"
#include "../list.h"
#include "../error/error.h"
#include "../io/log.h"

#include <stdarg.h>
#include <string.h>

#define GET_VALIDATOR(va) Validator_T* v = va_arg(va, Validator_T*)

// validator structs
typedef struct VALIDATOR_SCOPE_STRUCT VScope_T;
struct VALIDATOR_SCOPE_STRUCT
{
    VScope_T* prev;
    List_T* objs;
} __attribute__((packed));

typedef struct VALIDATOR_STRUCT 
{
    VScope_T* current_scope;
    int scope_depth;
} Validator_T;

// validator struct functions
static Validator_T* init_validator(Validator_T* v)
{
    v->scope_depth = 0;
    v->current_scope = NULL;

    return v;
}

static void begin_obj_scope(Validator_T* v, List_T* objs);
static void scope_add_obj(Validator_T* v, ASTObj_T* obj);
static inline void begin_scope(Validator_T* v);
static inline void end_scope(Validator_T* v);

// iterator functions

// id
static void id_use(ASTIdentifier_T* id, va_list args);
static void id_def(ASTIdentifier_T* id, va_list args);

// obj
static void fn_start(ASTObj_T* fn, va_list args);
static void fn_end(ASTObj_T* fn, va_list args);
static void namespace_start(ASTObj_T* namespace, va_list args);
static void namespace_end(ASTObj_T* namespace, va_list args);
static void typedef_start(ASTObj_T* tydef, va_list args);
static void typedef_end(ASTObj_T* tydef, va_list args);
static void global_start(ASTObj_T* global, va_list args);
static void global_end(ASTObj_T* global, va_list args);
static void local_start(ASTObj_T* local, va_list args);
static void local_end(ASTObj_T* local, va_list args);
static void fn_arg_start(ASTObj_T* arg, va_list args);
static void fn_arg_end(ASTObj_T* arg, va_list args);
static void va_list_start(ASTObj_T* va, va_list args);
static void va_list_end(ASTObj_T* va, va_list args);

// node
static void block_start(ASTNode_T* block, va_list args);
static void block_end(ASTNode_T* block, va_list args);
static void call(ASTNode_T* call, va_list args);

// iterator configuration
static ASTIteratorList_T main_iterator_list = 
{
    .node_start_fns = 
    {
        [ND_BLOCK] = block_start,
    },

    .node_end_fns = 
    {
        [ND_BLOCK] = block_end,
        [ND_CALL] = call,
    },

    .type_fns = 
    {

    },

    .obj_start_fns = 
    {
        [OBJ_FUNCTION] = fn_start,
        [OBJ_NAMESPACE] = namespace_start,
        [OBJ_TYPEDEF] = typedef_start,
        [OBJ_GLOBAL] = global_start,
        [OBJ_LOCAL] = local_start,
        [OBJ_FN_ARG] = fn_arg_start,
        [OBJ_VA_LIST] = va_list_start,
    },

    .obj_end_fns = 
    {
        [OBJ_FUNCTION] = fn_end,
        [OBJ_NAMESPACE] = namespace_end,
        [OBJ_TYPEDEF] = typedef_end,
        [OBJ_GLOBAL] = global_end,
        [OBJ_LOCAL] = local_end,
        [OBJ_FN_ARG] = fn_arg_end,
        [OBJ_VA_LIST] = va_list_end,
    },

    .id_def_fn = id_def,
    .id_use_fn = id_use
};

void validate_ast(ASTProg_T* ast)
{
    Validator_T v;
    init_validator(&v);

    begin_obj_scope(&v, ast->objs);
    ast_iterate(&main_iterator_list, ast, &v);

    end_scope(&v);
}

static ASTObj_T* search_in_current_scope(VScope_T* scope, char* id)
{
    for(size_t i = 0; i < scope->objs->size; i++)
    {
        ASTObj_T* obj = scope->objs->items[i];
        if(strcmp(obj->id->callee, id) == 0)
            return obj;
    }
    return NULL;
}

static ASTObj_T* search_in_scope(VScope_T* scope, char* id)
{
    if(!scope)
        return NULL;

    ASTObj_T* found = search_in_current_scope(scope, id);
    if(found)
        return found;
    return search_in_scope(scope->prev, id);
}

static ASTObj_T* search_identifier(VScope_T* scope, ASTIdentifier_T* id)
{
    if(!scope || !id)
        return NULL;

    if(!id->outer)
    {
        ASTObj_T* found = search_in_current_scope(scope, id->callee);
        if(found)
            return found;
        return search_identifier(scope->prev, id);
    }
    else {
        ASTObj_T* outer = search_in_scope(scope, id->outer->callee);
        if(!outer || !outer->objs)
            return NULL;
        
        for(size_t i = 0; i < outer->objs->size; i++)
        {
            ASTObj_T* current = outer->objs->items[i];
            if(strcmp(current->id->callee, id->callee) == 0)
                return current;
        }
        return NULL;
    }
}

static void begin_obj_scope(Validator_T* v, List_T* objs)
{
    begin_scope(v);

    for(size_t i = 0; i < objs->size; i++)
        scope_add_obj(v, objs->items[i]);
}

static inline void begin_scope(Validator_T* v)
{
    VScope_T* scope = malloc(sizeof(VScope_T));
    scope->objs = init_list(sizeof(ASTObj_T*));
    scope->prev = v->current_scope;
    v->current_scope = scope;
}

static inline void end_scope(Validator_T* v)
{
    VScope_T* scope = v->current_scope;
    v->current_scope = scope->prev;
    free_list(scope->objs);
    free(scope);
}

static void scope_add_obj(Validator_T* v, ASTObj_T* obj)
{
    ASTObj_T* found = search_in_current_scope(v->current_scope, obj->id->callee);
    if(found)
    {
        throw_error(ERR_REDEFINITION, obj->id->tok, "redefinition of %s `%s`.\nfirst defined in " COLOR_BOLD_WHITE "%s " COLOR_RESET "at line " COLOR_BOLD_WHITE "%lld" COLOR_RESET " as %s.", 
            obj_kind_to_str(obj->kind), obj->id->callee, 
            found->tok->source->short_path ? found->tok->source->short_path : found->tok->source->path, 
            found->tok->line + 1,
            obj_kind_to_str(found->kind)
        );
        exit(1);
    }
    list_push(v->current_scope->objs, obj);
}

// id

static void id_def(ASTIdentifier_T* id, va_list args)
{
}

static void id_use(ASTIdentifier_T* id, va_list args)
{
    GET_VALIDATOR(args);
    ASTObj_T* found = search_identifier(v->current_scope, id);
    if(!found)
        throw_error(ERR_SYNTAX_WARNING, id->tok, "undefined identifier `%s`.", id->callee);
}

// obj

static void fn_start(ASTObj_T* fn, va_list args)
{
    GET_VALIDATOR(args);
    begin_scope(v);
}

static void fn_end(ASTObj_T* fn, va_list args)
{
    GET_VALIDATOR(args);
    end_scope(v);
}

static void namespace_start(ASTObj_T* namespace, va_list args)
{
    GET_VALIDATOR(args);
    begin_obj_scope(v, namespace->objs);
}

static void namespace_end(ASTObj_T* namespace, va_list args)
{
    GET_VALIDATOR(args);
    end_scope(v);
}

static void typedef_start(ASTObj_T* tydef, va_list args)
{

}

static void typedef_end(ASTObj_T* tydef, va_list args)
{

}

static void global_start(ASTObj_T* global, va_list args)
{

}

static void global_end(ASTObj_T* global, va_list args)
{

}

static void local_start(ASTObj_T* local, va_list args)
{
   
}

static void local_end(ASTObj_T* local, va_list args)
{
}

static void fn_arg_start(ASTObj_T* arg, va_list args)
{
    GET_VALIDATOR(args);
    scope_add_obj(v, arg);
}

static void fn_arg_end(ASTObj_T* arg, va_list args)
{

}

static void va_list_start(ASTObj_T* va, va_list args)
{
    GET_VALIDATOR(args);
    scope_add_obj(v, va);
}

static void va_list_end(ASTObj_T* va, va_list args)
{

}

// node

static void block_start(ASTNode_T* block, va_list args)
{
    GET_VALIDATOR(args);
    begin_obj_scope(v, block->locals);
}

static void block_end(ASTNode_T* block, va_list args)
{
    GET_VALIDATOR(args);
    end_scope(v);
}

static void call(ASTNode_T* call, va_list args)
{
}