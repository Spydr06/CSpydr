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
    ast->innerType = NULL;

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