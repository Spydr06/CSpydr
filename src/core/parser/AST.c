#include "../../log.h"
#include "../token.h"
#include "AST.h"
#include <string.h>

ASTRoot_T* initASTRoot()
{
    ASTRoot_T* ast = calloc(1, sizeof(struct AST_ROOT_STRUCT));

    ast->functions = initList(sizeof(struct AST_FUNCTION_STRUCT*));
    ast->globals = initList(sizeof(struct AST_GLOBAL_STRUCT*));

    return ast;
}

ASTGlobal_T* initASTGlobal(char* name, ASTDataType_T* dataType, ASTExpr_T* value)
{
    ASTGlobal_T* ast = calloc(1, sizeof(struct AST_GLOBAL_STRUCT));

    ast->name = name;
    ast->dataType = dataType;
    ast->value = value;

    return ast;
}

ASTLocal_T* initASTLocal(char* name, ASTDataType_T* dataType, ASTExpr_T* value)
{
    ASTLocal_T* ast = calloc(1, sizeof(struct AST_LOCAL_STRUCT));

    ast->name = name;
    ast->dataType = dataType;
    ast->value = value;

    return ast;
}

ASTArgument_T* initASTArgument(char* name, ASTDataType_T* dataType)
{
    ASTArgument_T* ast = calloc(1, sizeof(struct AST_ARGUMENT_STRUCT));
    ast->name = name;
    ast->dataType = dataType;

    return ast;
}

ASTFunction_T* initASTFunction(char* name)
{
    ASTFunction_T* ast = calloc(1, sizeof(struct AST_FUNCTION_STRUCT));
    ast->name = name;
    ast->args = initList(sizeof(struct AST_ARGUMENT_STRUCT*));

    return ast;
}

ASTExpr_T* initASTExpr(ASTExprType_T type, void* value)
{
    ASTExpr_T* ast = calloc(1, sizeof(struct AST_EXPR_STRUCT));
    ast->type = type;
    ast->expr = value;

    return ast;
}

ASTExpr_T* initASTExprNil()
{
    ASTExpr_T* ast = calloc(1, sizeof(struct AST_EXPR_STRUCT));
    ast->type = AST_EX_NIL;
    ast->expr = NULL;

    return ast;
}

ASTDataType_T* initASTDataType_T(ASTBasicDataType_T dataType)
{
    ASTDataType_T* ast = calloc(1, sizeof(struct AST_DATA_TYPE));
    ast->basicType = dataType;
    ast->innerType = NULL;          //only used in arrays
    ast->numberOfIndices = NULL;    //only sometimes used in arrays

    return ast;
}

ASTCompound_T* initASTCompound()
{
    ASTCompound_T* ast = calloc(1, sizeof(struct AST_COMPOUND_STRUCT)); 
    ast->locals = initList(sizeof(struct AST_LOCAL_STRUCT*));
    ast->body = initList(sizeof(struct AST_COMPOUND_INSTRUCTION_STRUCT*));

    return ast;
}

ASTCompoundInstruction_T* initCompoundInstruction(ASTCompoundInstructionType_T type, void* ptr)
{
    ASTCompoundInstruction_T* ast = calloc(1, sizeof(struct AST_COMPOUND_INSTRUCTION_STRUCT));
    ast->type = type;
    ast->ptr = ptr;

    return ast;
}

ASTExprFnCall_T* initASTFunctionCall(char* callee)
{
    ASTExprFnCall_T* ast = calloc(1, sizeof(struct AST_EXPR_FN_CALL_STRUCT));

    ast->callee = callee;
    ast->args = initList(sizeof(struct AST_EXPR*));
    //TODO: get the datatype

    return ast;
}

ASTExprVarCall_T* initASTVarCall(char* callee)
{
    ASTExprVarCall_T* ast = calloc(1, sizeof(struct AST_EXPR_VAR_CALL_STRUCT));

    ast->callee = callee;
    //TODO: get the dataType;

    return ast;
}

ASTAssignment_T* initASTAssingment(ASTExprVarCall_T* target, ASTExpr_T* value)
{
    ASTAssignment_T* ast = calloc(1, sizeof(struct AST_ASSIGNMENT_STRUCT));

    ast->target = target;
    ast->value = value;
    //TODO: get the dataType;

    return ast;
}

ASTExitStmt_T* initASTExitStmt(ASTExpr_T* exitCode)
{
    ASTExitStmt_T* ast = calloc(1, sizeof(struct AST_EXIT_STMT_STRUCT));
    ast->exitCode = exitCode;

    return ast;
}

ASTReturnStmt_T* initASTReturnStmt(ASTExpr_T* returnValue)
{
    ASTReturnStmt_T* ast = calloc(1, sizeof(struct AST_RETURN_STMT_STRUCT));
    ast->returnValue = returnValue;
    
    return ast;
}

ASTIfStmt_T* initASTIfStmt(ASTExpr_T* condition)
{
    ASTIfStmt_T* ast = calloc(1, sizeof(struct AST_IF_STMT_STRUCT));
    ast->condition = condition;
    ast->elseBody = NULL,
    ast->ifBody = NULL;

    return ast;
}

ASTWhileStmt_T* initASTWhileStmt(ASTExpr_T* condition)
{
    ASTWhileStmt_T* ast = calloc(1, sizeof(struct AST_WHILE_STMT_STRUCT));
    ast->condition = condition;
    ast->body = NULL;

    return ast;
}

ASTForStmt_T* initASTForStmt(ASTLocal_T* counterVar, ASTExpr_T* condition, ASTAssignment_T* increment)
{
    ASTForStmt_T* ast = calloc(1, sizeof(struct AST_FOR_STMT_STRUCT));
    ast->condition = condition;
    ast->counterVar = counterVar;
    ast->increment = increment;
    ast->body = NULL;

    return ast;
}

ASTExprClosure_T* initASTClosure(ASTExpr_T* value)
{
    ASTExprClosure_T* ast = calloc(1, sizeof(struct AST_EXPR_CLOSURE_STRUCT));
    ast->value = value;
    //TODO: get the datatype

    return ast;
}

ASTExprConstant_T* initASTConstant(ASTDataType_T* dataType, void* ptr)
{
    ASTExprConstant_T* ast = calloc(1, sizeof(struct AST_EXPR_CLOSURE_STRUCT));
    ast->dataType = dataType;
    ast->value = ptr;

    return ast;
}

ASTExprNegate_T* initASTNegate(ASTExpr_T* expr)
{
    ASTExprNegate_T* ast = calloc(1, sizeof(struct AST_EXPR_STRUCT));
    //TODO: get the datatype
    ast->value = expr;

    return ast;
}

ASTExprNot_T* initASTNot(ASTExpr_T* expr)
{
    ASTExprNot_T* ast = calloc(1, sizeof(struct AST_EXPR_NOT_STRUCT));
    ast->dataType = initASTDataType_T(AST_BOOL);

    return ast;
}

ASTExprOp_T* initASTOp(ASTExprOpType_T type, ASTExpr_T* left, ASTExpr_T* right)
{
    ASTExprOp_T* ast = calloc(1, sizeof(struct AST_EXPR_OP_STRUCT));
    ast->type = type;
    ast->left = left;
    ast->right = right;
    //TODO: get the datatype

    return ast;
}

ASTExprBoolOp_T* initASTBoolOp(ASTExprBoolOpType_T type, ASTExpr_T* left, ASTExpr_T* right)
{
    ASTExprBoolOp_T* ast = calloc(1, sizeof(struct AST_EXPR_BOOL_OP_STRUCT));
    ast->dataType = initASTDataType_T(AST_BOOL);
    ast->left = left; 
    ast->right = right;
    ast->type = type;

    return ast;
}

ASTExprArray_T* initASTArrayExpr()
{
    ASTExprArray_T* ast = calloc(1, sizeof(struct AST_EXPR_ARRAY_STRUCT));
    ast->dataType = initASTDataType_T(AST_ARRAY);
    ast->indices = initList(sizeof(struct AST_EXPR_STRUCT*));
    //TODO: get the indices data type

    return ast;
}

static void freeASTCompound(ASTCompound_T* ast);
static void freeASTVarCall(ASTExprVarCall_T* ast);
static void freeASTFnCall(ASTExprFnCall_T* ast);
static void freeASTExpr(ASTExpr_T* ast);

static void freeASTConstant(ASTExprConstant_T* ast)
{   if(ast == NULL) 
        return;

    free(ast->value);
    free(ast);
}

static void freeASTDataType(ASTDataType_T* ast)
{
    if(ast == NULL) 
        return;

    freeASTDataType(ast->innerType);
    freeASTConstant(ast->numberOfIndices);

    free(ast);
}

static void freeASTArrayExpr(ASTExprArray_T* ast)
{
    if(ast == NULL) 
        return;

    freeASTDataType(ast->dataType);
    
    for(int i = 0; i < ast->indices->size; i++)
    {
        freeASTExpr(ast->indices->items[i]);
    }
    freeList(ast->indices);
    free(ast);
}

static void freeASTClosure(ASTExprClosure_T* ast)
{
    if(ast == NULL) 
        return;

    freeASTDataType(ast->dataType);
    freeASTExpr(ast->value);

    free(ast);
}

static void freeASTNot(ASTExprNot_T* ast)
{
    if(ast == NULL) 
        return;

    freeASTDataType(ast->dataType);
    freeASTExpr(ast->value);

    free(ast);
}

static void freeASTNegate(ASTExprNegate_T* ast)
{
    if(ast == NULL) 
        return;

    freeASTDataType(ast->dataType);
    freeASTExpr(ast->value);

    free(ast);
}

static void freeASTOp(ASTExprOp_T* ast)
{
    if(ast == NULL) 
        return;
    
    freeASTDataType(ast->dataType);
    freeASTExpr(ast->left);
    freeASTExpr(ast->right),

    free(ast);
}

static void freeASTBoolOp(ASTExprBoolOp_T* ast)
{
    if(ast == NULL)
        return;
    
    freeASTDataType(ast->dataType);
    freeASTExpr(ast->left);
    freeASTExpr(ast->right),

    free(ast);
}

static void freeASTExpr(ASTExpr_T* ast)
{
    if(ast == NULL) 
        return;
    
    switch(ast->type)
    {
        case AST_EX_CONSTANT:
            freeASTConstant(ast->expr);
            break;
        case AST_EX_ARRAY:
            freeASTArrayExpr(ast->expr);
            break;
        case AST_EX_VAR_CALL:
            freeASTVarCall(ast->expr);
            break;
        case AST_EX_FN_CALL:
            freeASTFnCall(ast->expr);
            break;
        case AST_EX_CLOSURE:
            freeASTClosure(ast->expr);
            break;
        case AST_EX_OP:
            freeASTOp(ast->expr);
            break;
        case AST_EX_NIL:    //nothing to do here since there is no AST type for "nil"
            break;
        case AST_EX_BOOL_OP:
            freeASTBoolOp(ast->expr);
            break;
        case AST_EX_NOT:
            freeASTNot(ast->expr);
            break;
        case AST_EX_NEGATE:
            freeASTNegate(ast->expr);
            break;
    }
    free(ast);
}

static void freeASTGlobal(ASTGlobal_T* ast)
{
    if(ast == NULL) 
        return;

    freeASTDataType(ast->dataType);
    freeASTExpr(ast->value);

    free(ast);
}

static void freeASTArgument(ASTArgument_T* ast)
{
    if(ast == NULL) 
        return;
    
    freeASTDataType(ast->dataType);

    free(ast);
}

static void freeASTLocal(ASTLocal_T* ast)
{
    if(ast == NULL) 
        return;

    freeASTDataType(ast->dataType);
    freeASTExpr(ast->value);

    free(ast);
}

static void freeASTFnCall(ASTExprFnCall_T* ast)
{
    if(ast == NULL) 
        return;

    for(int i = 0; i < ast->args->size; i++)
    {
        freeASTExpr(ast->args->items[i]);
    }
    freeList(ast->args);
    freeASTDataType(ast->dataType);
    free(ast);
}

static void freeASTVarCall(ASTExprVarCall_T* ast)
{
    if(ast == NULL) 
        return;

    freeASTDataType(ast->dataType);
    free(ast);
}

static void freeASTAssignment(ASTAssignment_T* ast)
{
    if(ast == NULL) 
        return;

    freeASTVarCall(ast->target);
    freeASTDataType(ast->dataType);
    freeASTExpr(ast->value);

    free(ast);
}

static void freeASTWhile(ASTWhileStmt_T* ast)
{
    if(ast == NULL) 
        return;
    
    freeASTCompound(ast->body);
    freeASTExpr(ast->condition);

    free(ast);
}

static void freeASTFor(ASTForStmt_T* ast)
{
    if(ast == NULL) 
        return;

    freeASTLocal(ast->counterVar);
    freeASTExpr(ast->condition);
    freeASTAssignment(ast->increment);
    freeASTCompound(ast->body);

    free(ast);
}

static void freeASTIf(ASTIfStmt_T* ast)
{
    if(ast == NULL) 
        return;

    freeASTExpr(ast->condition);
    freeASTCompound(ast->ifBody);
    freeASTCompound(ast->elseBody);

    free(ast);
}

static void freeASTReturn(ASTReturnStmt_T* ast)
{
    if(ast == NULL) 
        return;

    freeASTExpr(ast->returnValue);
    free(ast);
}

static void freeASTExit(ASTExitStmt_T* ast)
{
    if(ast == NULL) 
        return;

    freeASTExpr(ast->exitCode);
    free(ast);
}

static void freeASTCompoundInstruction(ASTCompoundInstruction_T* ast)
{
    if(ast == NULL) 
        return;

    switch(ast->type)
    {
        case AST_CI_LOCALDEF:
            freeASTLocal(ast->ptr);
            break;
        case AST_CI_ASSIGN:
            freeASTAssignment(ast->ptr);
            break;
        case AST_CI_WHILE_STMT:
            freeASTWhile(ast->ptr);
            break;
        case AST_CI_FOR_STMT:
            freeASTFor(ast->ptr);
            break;
        case AST_CI_IF_STMT:
            freeASTIf(ast->ptr);
            break;
        case AST_CI_RETURN_STMT:
            freeASTReturn(ast->ptr);
            break;
        case AST_CI_EXIT_STMT:
            freeASTExit(ast->ptr);
            break;
        case AST_CI_FN_CALL:
            freeASTFnCall(ast->ptr);
            break;
    }

    free(ast);
}

static void freeASTCompound(ASTCompound_T* ast)
{
    if(ast == NULL) 
        return;

    for(int i = 0; i < ast->body->size; i++)
    {
        freeASTCompoundInstruction(ast->body->items[i]);    
    }
    freeList(ast->body);
    free(ast);
}

static void freeASTFunction(ASTFunction_T* ast)
{
    if(ast == NULL) 
        return;

    for(int i = 0; i < ast->args->size; i++)
    {
        freeASTArgument(ast->args->items[i]);
    }
    freeList(ast->args);

    freeASTDataType(ast->returnType);
    freeASTCompound(ast->body);

    free(ast);
}

void freeAST(ASTRoot_T* ast)
{
    if(ast == NULL) 
        return;

    for(int i = 0; i < ast->globals->size; i++)
    {
        freeASTGlobal(ast->globals->items[i]);
    }
    freeList(ast->globals);

    for(int i = 0; i < ast->functions->size; i++)
    {
        freeASTFunction(ast->functions->items[i]);
    }
    freeList(ast->functions);
    //free(ast);
}