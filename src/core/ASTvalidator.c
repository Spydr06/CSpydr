#include "ASTvalidator.h"
#include "list.h"
#include "../log.h"
#include <string.h>

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

static void checkForDuplicateVariables(validator_T* validator);

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
            validateStmt(validator, ast);
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
        default:
            LOG_ERROR("No AST node with type '%d' found.\n", ast->type);
            exit(1);
    }
}

static void validateRoot(validator_T* validator, AST_T* ast)
{
    ASSERT(ast->root, "Root AST node is null%s", "\n");
    ASSERT(ast->root->contents, "Program file is empty.%s", "\n");
    ASSERT(ast->root->contents->size, "Program file is empty.%s", "\n");
    for(int i = 0; i < ast->root->contents->size; i++)
    {
        AST_T* currentAST = (AST_T*) ast->root->contents->items[i];

        if(currentAST->type == DEF) {
            validator->enclosingAST = ast;
            validateAST(validator, currentAST);
        }
        else {
            LOG_ERROR("Unexpected AST type %d\n", currentAST->type);
            exit(1);
        }
    }

    ast->validated = true;
}

static void validateStmt(validator_T* validator, AST_T* ast)
{
    ASSERT(ast->stmt, "Stmt AST node is null%s", "\n");

    switch(ast->stmt->type)
    {
        case RETURN:
            validateExpr(validator, ast->stmt->value);
            break;
        case EXIT:
            validateExpr(validator, ast->stmt->value);
            break;
        case FOR:
            validateDef(validator, ast->stmt->value);
            validateExpr(validator, ast->stmt->condition);
            validateExpr(validator, ast->stmt->inc);
            validateCompound(validator, ast->stmt->body);
            break;
        case WHILE:
            validateExpr(validator, ast->stmt->condition);
            validateCompound(validator, ast->stmt->body);
            break;
        case IF:
            validateExpr(validator, ast->stmt->condition);
            validateCompound(validator, ast->stmt->ifBody);
            if(ast->stmt->elseBody != NULL)
            {
                validateCompound(validator, ast->stmt->elseBody);
            }
            break;
    }

    ast->validated = true;
}

static void validateExpr(validator_T* validator, AST_T* ast)
{
    ast->validated = true;
}

static void validateDef(validator_T* validator, AST_T* ast)
{
    ASSERT(ast->def, "Definition AST node is null%s", "\n");
    ASSERT(ast->def->name, "Unnamed Variable definition in %s", validator->enclosingAST->type == ROOT ? "root" : "compound");
    
    if(ast->def->isFunction == true)
    {
        listPush(validator->functions, ast);
        ASSERT(ast->def->value, "Function has no body%s", "\n");
        //FIXME: results in segfault: validateCompound(validator, ast->def->value);
    }   
    else 
    {
        listPush(validator->variables, ast);
    }

    ast->validated = true;
}

static void validateCompound(validator_T* validator, AST_T* ast)
{
    ASSERT(ast->compound, "Compound AST node is null%s\n", "\n");
    ASSERT(ast->compound->contents, "Compound contents are null%s\n", "\n");

    int numOfVars = 0;

    for(int i = 0; i < ast->compound->contents->size; i++)
    {
        AST_T* currentAST = ast->compound->contents->items[i];
        ASSERT(ast, "Child AST of compound is null%s", "\n");

        switch(currentAST->type)
        {
            case DEF:
                numOfVars++;
                listPush(validator->variables, currentAST);
                validateDef(validator, currentAST);
                break;
            default:
                break;
        }
    }

    checkForDuplicateVariables(validator);
    validator->variables->size -= numOfVars;
    
    ast->validated = true;
}

static void checkForDuplicateVariables(validator_T* validator)
{
    for(int i = 0; i < validator->variables->size; i++)
    {
        for(int j = 0; j < validator->variables->size; j++)
        {
            AST_T* a = validator->variables->items[i];
            AST_T* b = validator->variables->items[j];
            if((i != j) && strcmp(a->def->name, b->def->name) == 0)
            {
                LOG_ERROR("Variable with the name '%s' already exists in current scope.\n", a->def->name);
                exit(1);
            }
        }
    }
}