#include "validator.h"

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

void validate_ast(ASTProg_T* ast)
{
    Validator_T* v = init_validator();

    // validate all types and check the whole code

    free_validator(v);
}