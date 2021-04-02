#include "BCCompiler.h"
#include "../log.h"

#include <stdbool.h>
#include <string.h>

#include "instructions.h"

BCCompiler_T* initBCCompiler()
{
    BCCompiler_T* compiler = calloc(1, sizeof(struct BYTECODE_COMPILER_STRUCT));
    compiler->instructions = initList(sizeof(struct BYTECODE_INSTRUCTION_STRUCT));

    return compiler;
}

static void submitInstruction(BCCompiler_T* compiler, BCInstruction_T* instruction);
static void compileLocalDefinition(BCCompiler_T* compiler, AST_T* ast);
static void compileGlobalDefinition(BCCompiler_T* compiler, AST_T* ast);
static void compileExpression(BCCompiler_T* compiler, AST_T* ast, unsigned int OPRegister);
static void compileStatement(BCCompiler_T* compiler, AST_T* ast);
static void compileFunction(BCCompiler_T* compiler, AST_T* ast);
static void compileAssignment(BCCompiler_T* compiler, AST_T* ast, unsigned int OPRegister);
static void compileConstants(BCCompiler_T* compiler, AST_T* ast, unsigned int OPRegister);
static char* compileCall(BCCompiler_T* compiler, AST_T* ast, unsigned int OPRegister);
static void compileOperation(BCCompiler_T* compiler, AST_T* ast, unsigned int OPRegister);

void compileBC(BCCompiler_T* compiler, AST_T* root)
{
    LOG_OK(COLOR_BOLD_GREEN "Generating" COLOR_RESET " bytecode\t");

    for(int i = 0; i < root->root->contents->size; i++)
    {
        AST_T* currentAST = (AST_T*) root->root->contents->items[i];

        if(currentAST->type != DEF) 
        {
            LOG_ERROR(COLOR_BOLD_RED "The root AST can only hold definitions.\n");
            exit(1);
        }

        if(currentAST->def->isFunction)
        {
            compileFunction(compiler, currentAST);
        } else
        {
            compileGlobalDefinition(compiler, currentAST);
        }
    }

    LOG_OK("done!\n");
}

static void compileCompound(BCCompiler_T* compiler, AST_T* ast)
{
    for(int i = 0; i < ast->compound->contents->size; i++)
    {
        AST_T* currentAST = ast->compound->contents->items[i];

        switch(currentAST->type)
        {
            case ROOT:
                LOG_ERROR(COLOR_BOLD_RED "Root ast found in compound.\n");
                exit(1);
            case EXPR:
                compileExpression(compiler, currentAST, 0);
                break;
            case STMT:
                compileStatement(compiler, currentAST);
                break;
            case DEF:
                compileLocalDefinition(compiler, currentAST);
                break;
            case COMPOUND:
                compileCompound(compiler, ast);
                break;
            default:
                LOG_ERROR_F(COLOR_BOLD_RED "Undefine ast type '%d' found in compound.\n", currentAST->type);
                exit(1);
        }
    }
}

static void compileStatement(BCCompiler_T* compiler, AST_T* ast)
{
    switch(ast->stmt->type)
    {
        case RETURN:
            compileExpression(compiler, ast->stmt->value, 0);
            submitInstruction(compiler, initInstruction1(OP_RET, "%0"));
            break;
        case EXIT:
            compileExpression(compiler, ast->stmt->value, 0);
            submitInstruction(compiler, initInstruction1(OP_EXIT, "%0"));
            break;
        case IF:
            //FIXME: compileExpression(compiler, ast->stmt->condition, 0);
            submitInstruction(compiler, initInstruction1(OP_IF, "%0"));
            compileCompound(compiler, ast->stmt->ifBody);

            if(ast->stmt->elseBody != NULL)
            {
                submitInstruction(compiler, initInstruction0(OP_ELSE));
                compileCompound(compiler, ast->stmt->elseBody);
            }
            submitInstruction(compiler, initInstruction0(OP_IF_END));
            break;
        default:
            LOG_ERROR_F("Unknown statement type '%d'\n", ast->expr->type);
            exit(1);  
    }
}

static void compileExpression(BCCompiler_T* compiler, AST_T* ast, unsigned int OPRegister)
{
    switch(ast->expr->type)
    {
        case CONSTANT:
            compileConstants(compiler, ast, OPRegister);
            break;

        case ASSIGN:
            compileAssignment(compiler, ast, OPRegister);
            break;

        case CALL:
            compileCall(compiler, ast, OPRegister);
            break;

        case ADD:
        case SUB:
        case MULT:
        case DIV:
            compileOperation(compiler, ast, OPRegister);
            break;

        default:
            LOG_ERROR_F("Unknown expression type '%d'\n", ast->expr->type);
            exit(1);
    }
}

static void compileOperation(BCCompiler_T* compiler, AST_T* ast, unsigned int OPRegister)
{
}

static void compileAssignment(BCCompiler_T* compiler, AST_T* ast, unsigned int OPRegister)
{
    const char* template = "@%s";
    char* name = calloc(strlen(ast->expr->op.left->expr->name) + strlen(template) + 1, sizeof(char));
    sprintf(name, template, ast->expr->op.left->expr->name);

    compileExpression(compiler, ast->expr->op.right, OPRegister + 1);

    submitInstruction(compiler, initInstruction2(OP_SET, name, "%a"));
}

static char* compileCall(BCCompiler_T* compiler, AST_T* ast, unsigned int OPRegister)
{
    char* reg = calloc(12 + 2, sizeof(char));
    sprintf(reg, "%%%d", OPRegister);

    if(ast->expr->isFunctionCall) 
    {
        
    }
    else {
        submitInstruction(compiler, initInstruction2(OP_SET, reg, ast->expr->name));
    }

    return ast->expr->name;
}

static void compileConstants(BCCompiler_T* compiler, AST_T* ast, unsigned int OPRegister)
{
    char* reg = calloc(12 + 2, sizeof(char));
    sprintf(reg, "%%%d", OPRegister);
    
    
    switch(ast->expr->dataType->dataType->type)
    {
        case I8:
        case I16:
        case I32:
        case U8:
        case U16:
        case U32:
        case U64:
        case I64: {
            char* value = calloc(17, sizeof(char));
            sprintf(value, "%di", ast->expr->intValue);
            submitInstruction(compiler, initInstruction2(OP_SET, reg, value));
        } break;
        case F32:
        case F64: {
            char* value = calloc(17, sizeof(char));
            sprintf(value, "%ff", ast->expr->floatValue);
            submitInstruction(compiler, initInstruction2(OP_SET, reg, value));
        } break;
        
        case STR: {
            char* value = calloc(strlen(ast->expr->strValue) + 3, sizeof(char));
            sprintf(value, "\"%s\"", ast->expr->strValue);
            submitInstruction(compiler, initInstruction2(OP_SET, reg, value));
        } break;
        case BOOL: {
            submitInstruction(compiler, initInstruction2(OP_SET, reg, ast->expr->boolValue ? "true" : "false"));
        } break;
        case CHAR: {
            char* value = calloc(4, sizeof(char));
            sprintf(value, "\'%c\'", ast->expr->charValue);
            submitInstruction(compiler, initInstruction2(OP_SET, reg, value));
        } break;
        case VOID: {
            submitInstruction(compiler, initInstruction2(OP_SET, reg, "void"));
        }
        case VEC: {
            //TODO: DO THIS
        } break;
    }
}

static void compileFunction(BCCompiler_T* compiler, AST_T* ast)
{
    submitInstruction(compiler, initInstruction2(OP_FN, ast->def->name, dataTypeToString(ast->def->dataType)));
             
    for(int i = 0; i < ast->def->args->size; i++)
    {
        AST_T* arg = ast->def->args->items[i];
        
        compileLocalDefinition(compiler, arg);
        const char* template = "@%s";
        char* value = calloc(strlen(template) + strlen(arg->def->name) + 1, sizeof(char));
        sprintf(value, template, arg->def->name);
        submitInstruction(compiler, initInstruction1(OP_ARG, value));
    }
    if(ast->def->value->type != COMPOUND)
    {
        LOG_ERROR(COLOR_BOLD_RED "Function value has to be a compound.\n");
        exit(1);
    }
    compileCompound(compiler, ast->def->value);
}

static void compileLocalDefinition(BCCompiler_T* compiler, AST_T* ast)
{
    submitInstruction(compiler, initInstruction2(OP_LOCAL, ast->def->name, dataTypeToString(ast->def->dataType)));
    if(ast->def->value != NULL)
    {
        const char* template = "@%s";
        char* value = calloc(strlen(template) + strlen(ast->def->name) + 1, sizeof(char));
        sprintf(value, template, ast->def->name);

        compileExpression(compiler, ast->def->value, 0);
        submitInstruction(compiler, initInstruction2(OP_SET, value, "%0"));
    }
}

static void compileGlobalDefinition(BCCompiler_T* compiler, AST_T* ast)
{
    submitInstruction(compiler, initInstruction2(OP_GLOBAL, ast->def->name, dataTypeToString(ast->def->dataType)));
    if(ast->def->value != NULL)
    {
        const char* template = "@%s";
        char* value = calloc(strlen(template) + strlen(ast->def->name) + 1, sizeof(char));
        sprintf(value, template, ast->def->name);

        compileExpression(compiler, ast->def->value, 0);
        submitInstruction(compiler, initInstruction2(OP_SET, value, "%0"));
    }
}

static void submitInstruction(BCCompiler_T* compiler, BCInstruction_T* instruction)
{
    listPush(compiler->instructions, instruction);
}