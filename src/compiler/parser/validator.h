#ifndef CSPYDR_VALIDATOR_H
#define CSPYDR_VALIDATOR_H

#include "ast/ast.h"

// validator structs
typedef struct VALIDATOR_SCOPE_STRUCT VScope_T;
struct VALIDATOR_SCOPE_STRUCT
{
    VScope_T* prev;
    List_T* objs;
    ASTIdentifier_T* id;
} __attribute__((packed));

typedef struct VALIDATOR_STRUCT 
{
    ASTProg_T* ast;

    VScope_T* current_scope;
    VScope_T* global_scope;
    ASTObj_T* current_function;
    ASTNode_T* current_pipe;
    bool main_function_found;

    u32 scope_depth;  // depth of the current scope
} Validator_T;

void validate_ast(ASTProg_T* ast);
ASTType_T* expand_typedef(Validator_T* v, ASTType_T* type);

#endif