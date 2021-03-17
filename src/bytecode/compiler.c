#include "compiler.h"
#include "../log.h"

BCCompiler_T* initBCCompiler()
{
    BCCompiler_T* compiler = calloc(1, sizeof(struct BYTECODE_COMPILER_STRUCT));
    compiler->instructions = initList(sizeof(struct BYTECODE_INSTRUCTION_STRUCT));

    return compiler;
}

static void submitInstruction(BCCompiler_T* compiler, BCInstruction_T* instruction);

void compileBC(BCCompiler_T* compiler, AST_T* root)
{
    LOG_OK(COLOR_BOLD_GREEN "Generating" RESET " bytecode%s", "\t");

    for(int i = 0; i < root->root->contents->size; i++)
    {
        AST_T* currentAST = root->root->contents->items[i];

        if(currentAST->type != DEF) 
        {
            LOG_ERROR(COLOR_BOLD_RED "The root AST can only hold definitions.%s", "\n");
            exit(1);
        }

        if(!currentAST->def->isFunction)
        {
            submitInstruction(compiler, initInstruction(OP_GLOBAL, 1, currentAST->def->name));
        }
    }

    LOG_OK("done!%s", "\n");
}

static void submitInstruction(BCCompiler_T* compiler, BCInstruction_T* instruction)
{
    listPush(compiler->instructions, instruction);
}