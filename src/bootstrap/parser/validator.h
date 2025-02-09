#ifndef CSPYDR_VALIDATOR_H
#define CSPYDR_VALIDATOR_H

#include "ast/ast.h"
#include "config.h"
#include "hashmap.h"
#include "list.h"
#include "optimizer/constexpr.h"
#include "parser/typechecker.h"

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

    TypeChecker_T typechecker;

    Scope_T* current_scope;
    Scope_T* global_scope;

    ASTObj_T* current_obj;
    List_T* obj_stack;

    ConstexprResolver_T constexpr_resolver;

    bool main_function_found;

    bool need_exact_type_infos;
    List_T* exact_type_info_stack;
        
    ASTNode_T* current_pipe;

    u32 scope_depth;  // depth of the current scope
} Validator_T;

i32 validator_pass(Context_T* context, ASTProg_T* ast);

void validator_push_obj(Validator_T* v, ASTObj_T* obj);
void validator_pop_obj(Validator_T* v);

void validator_push_type_info(Validator_T* v, bool need_exact);
void validator_pop_type_info(Validator_T* v);

#endif
