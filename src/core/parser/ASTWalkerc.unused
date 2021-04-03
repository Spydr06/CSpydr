#include "ASTWalker.h"
#include "../../log.h"
#include "ACT.h"

static ACTCompound_T* generateCompoundACT(ACTCompound_T* enclosing, AST_T* ast);

ACTRoot_T* generateActionTree(AST_T* ast)
{
    LOG_OK(COLOR_BOLD_GREEN "Generating" COLOR_RESET " Action Tree\n");

    if(ast->type != ROOT)
    {
        LOG_ERROR_F(COLOR_BOLD_RED "Error while generating Action Tree:\n" COLOR_RESET COLOR_RED "Expected ast of type 'ROOT', got type '%d'!\n", ast->type);
        exit(1);
    }

    ACTRoot_T* root = initACTRoot();

    for(int i = 0; i < ast->root->contents->size; i++)
    {
        AST_T* currentAST = (AST_T*) ast->root->contents->items[i];
        if(currentAST->type == DEF)
        {
            if(currentAST->def->isFunction)
            {
                registerFunction(root, currentAST);
            } else 
            {   
                registerGlobal(root, currentAST);
            }
        }
    }

    for(int i = 0; i < root->globals->size; i++)
    {
        //ACTGlobal_T* currentGlobal = root->globals->items[i];

        //TODO: assign the expressions
    }

    for(int i = 0; i < root->functions->size; i++)
    {
        ACTFunction_T* currentFunc = (ACTFunction_T*) root->functions->items[i];

        //TODO: currentFunc->args
        //TODO: currentFunc->body = generateCompoundACT(NULL, ast->root->contents->items[1]);
    }

    return root;
}

static ACTCompound_T* generateCompoundACT(ACTCompound_T* enclosing, AST_T* ast)
{

}