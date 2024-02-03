#ifndef CSPYDR_VALIDATOR_H
#define CSPYDR_VALIDATOR_H

#include "ast/ast.h"
#include "config.h"
#include "hashmap.h"
#include "optimizer/constexpr.h"

// validator structs
typedef struct SCOPE_STRUCT Scope_T;
struct SCOPE_STRUCT
{
    Scope_T* outer;
    HashMap_T* objs;
};

typedef struct VALIDATOR_STRUCT 
{
    Context_T* context;
    ASTProg_T* ast;

    Scope_T* current_scope;
    Scope_T* global_scope;

    ASTObj_T* current_obj;
    List_T* obj_stack;

    ConstexprResolver_T constexpr_resolver;

    u32 scope_depth;  // depth of the current scope
} Validator_T;

i32 validator_pass(Context_T* context, ASTProg_T* ast);

#endif
