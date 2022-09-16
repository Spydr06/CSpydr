#ifndef CSPYDR_VALIDATOR_H
#define CSPYDR_VALIDATOR_H

#include "ast/ast.h"
#include "hashmap.h"

// validator structs
typedef struct SCOPE_STRUCT Scope_T;
struct SCOPE_STRUCT
{
    Scope_T* prev;
    HashMap_T* objs;
    ASTIdentifier_T* id;
} __attribute__((packed));

typedef struct VALIDATOR_STRUCT 
{
    ASTProg_T* ast;

    Scope_T* current_scope;
    Scope_T* global_scope;
    ASTObj_T* current_function;
    ASTNode_T* current_pipe;
    bool main_function_found;

    u32 scope_depth;  // depth of the current scope
} Validator_T;

void validate_ast(ASTProg_T* ast);
ASTType_T* expand_typedef(Validator_T* v, ASTType_T* type);

#endif