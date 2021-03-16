#include "ASTvalidator.h"
#include "list.h"
#include "log.h"

#define ASSERT(val, msg, ...) if(!val) {LOG_ERROR(msg, __VA_ARGS__); exit(1);}

#define MAX_NAME_LENGTH 128

validator_T* initASTValidator()
{
    validator_T* validator = calloc(1, sizeof(struct VALIDATOR_STRUCT));
    validator->functions = initList(MAX_NAME_LENGTH * sizeof(char));
    validator->variables = initList(MAX_NAME_LENGTH * sizeof(char));

    validator->numOfGlobalFns = 0;
    validator->numOfGlobalVars = 0;

    return validator;
}

static void validateRoot(validator_T* validator, AST_T* ast);
static void validateStmt(validator_T* validator, AST_T* ast);
static void validateExpr(validator_T* validator, AST_T* ast);
static void validateDef(validator_T* validator, AST_T* ast);
static void validateCompound(validator_T* validator, AST_T* ast);

void validateAST(validator_T* validator, AST_T* ast)
{
    switch(ast->type)
    {
        case STMT:
            validateAST(validator, ast);
            break;
        case EXPR:
            validateExpr(validator, ast);
            break;
        case DEF:
            validateDef(validator, ast);
            break;
        case COMPOUND:
            validateCompound(validator, ast);
            break;
        case ROOT:
            validateRoot(validator, ast);
            break;
    }
}

static void validateRoot(validator_T* validator, AST_T* ast)
{
    ASSERT(ast->root, "Fatal internal error: root AST node is null%s", "\n");
    ASSERT(ast->root->contents, "Program file is emty.%s", "\n");
    ASSERT(ast->root->contents->size, "Program file is emty.%s", "\n");

    for(int i = 0; i < ast->root->contents->size; i++)
    {
        AST_T* currentAST = (AST_T*) ast->root->contents->items[i];

        if(currentAST->type == DEF) {
            validator->enclosingAST = ast;
            validateAST(validator, currentAST);
        }
        else {
            LOG_ERROR("Unexpected AST type %d\n", currentAST->type);
        }
    }
}

static void validateStmt(validator_T* validator, AST_T* ast)
{

}

static void validateExpr(validator_T* validator, AST_T* ast)
{

}

static void validateDef(validator_T* validator, AST_T* ast)
{
    ASSERT(ast->def, "Fatal internal error: definition AST node is null%s", "\n");
    ASSERT(ast->def->name, "Unnamed Variable definition in %s", validator->enclosingAST->type == ROOT ? "root" : "compound");
    
    if(ast->def->isFunction)
    {
        
    }   
    else 
    {
        
    }
}

static void validateCompound(validator_T* validator, AST_T* ast)
{
    int numOfVars = 0;
    int numOfFns = 0;
}