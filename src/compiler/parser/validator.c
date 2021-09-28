#include "validator.h"
#include "../ast/ast_iterator.h"

// validator structs
struct VALIDATOR_SCOPE_STRUCT
{
    VScope_T* prev;
    List_T* objs;
};

struct VALIDATOR_STRUCT 
{
    VScope_T* current_scope;
    int scope_depth;
};

// validator struct functions
static Validator_T* init_validator()
{
    Validator_T* v = malloc(sizeof(struct VALIDATOR_STRUCT));
    v->scope_depth = 0;
    v->current_scope = NULL;

    return v;
}

static void free_validator(Validator_T* v)
{
    free(v);
}

// iterator functions
static void resolve_id_use(ASTIdentifier_T* id, va_list custom_args);
static void resolve_id_def(ASTIdentifier_T* id, va_list custom_args);

// iterator configuration
static ASTIteratorList_T iterator_list = 
{
    .node_fns = {

    },
    .type_fns = {

    },
    .obj_start_fns = {

    },
    .obj_end_fns = {

    },
    .id_def_fn = resolve_id_def,
    .id_use_fn = resolve_id_use
};

void validate_ast(ASTProg_T* ast)
{
    Validator_T* v = init_validator();

    ast_iterate(&iterator_list, ast, v);

    free_validator(v);
}

static void resolve_id_def(ASTIdentifier_T* id, va_list custom_args)
{

}

static void resolve_id_use(ASTIdentifier_T* id, va_list custom_args)
{
    
}