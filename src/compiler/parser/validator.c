#include "validator.h"
#include "../ast/ast_iterator.h"
#include "../list.h"
#include "../error/error.h"
#include "../io/log.h"

#include <stdarg.h>
#include <string.h>

#define GET_VALIDATOR(va) Validator_T* v = va_arg(va, Validator_T*)

typedef enum VSCOPE_KIND_ENUM
{
    V_MAIN,      // main scope (global)
    V_NAMESPACE, // scope in a namespace
    V_FUNCTION,  // scope in a function (arguments, etc.)
    V_BLOCK,     // scope in a block
    V_FOR,       // scope in a for loop
    V_LAMBDA,    // scope in a lambda expression

} VScopeKind_T;

// validator structs
typedef struct VALIDATOR_SCOPE_STRUCT VScope_T;
struct VALIDATOR_SCOPE_STRUCT
{
    VScopeKind_T kind;
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

// iterator configuration
static ASTIteratorList_T main_iterator_list = 
{
    .node_fns = 
    {

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
        [OBJ_FN_ARG] = fn_arg_start
    },

    .obj_end_fns = 
    {
        [OBJ_FUNCTION] = fn_end,
        [OBJ_NAMESPACE] = namespace_end,
        [OBJ_TYPEDEF] = typedef_end,
        [OBJ_GLOBAL] = global_end,
        [OBJ_LOCAL] = local_end,
        [OBJ_FN_ARG] = fn_arg_end
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

static ASTObj_T* search_in_scope(VScope_T* scope, char* id)
{
    for(int i = 0; i < scope->objs->size; i++)
    {
        ASTObj_T* obj = scope->objs->items[i];
        if(strcmp(obj->id->callee, id) == 0)
            return obj;
    }

    if(scope->prev)
        return search_in_scope(scope->prev, id);
    return NULL;
}

static void begin_obj_scope(Validator_T* v, List_T* objs)
{
    if(!objs)
        return;

    VScope_T* scope = malloc(sizeof(VScope_T));

    scope->prev = v->current_scope;
    v->current_scope = scope;
    scope->objs = init_list(sizeof(ASTObj_T*));

    for(size_t i = 0; i < objs->size; i++)
    {
        ASTObj_T* obj = objs->items[i];
        ASTObj_T* found = search_in_scope(scope, obj->id->callee);
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
        list_push(scope->objs, obj);
    }
}

static inline void end_scope(Validator_T* v)
{
    VScope_T* scope = v->current_scope;
    free_list(scope->objs);
    v->current_scope = scope->prev;
    free(scope);
}

// id

static void id_def(ASTIdentifier_T* id, va_list args)
{
    GET_VALIDATOR(args);
}

static void id_use(ASTIdentifier_T* id, va_list args)
{
    GET_VALIDATOR(args);
}

// obj

static void fn_start(ASTObj_T* fn, va_list args)
{
    GET_VALIDATOR(args);
}

static void fn_end(ASTObj_T* fn, va_list args)
{
    GET_VALIDATOR(args);
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

}

static void fn_arg_end(ASTObj_T* arg, va_list args)
{

}