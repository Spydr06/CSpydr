#ifndef CSPYDR_ASTION_TREE_H
#define CSPYDR_ASTION_TREE_H

#include "../list.h"

typedef enum AST_BASIC_DATA_TYPE
{
    AST_I32,
    AST_I64,

    AST_U32,
    AST_U64,

    AST_F32,
    AST_F64,

    AST_VOID,
    AST_BOOL,
    AST_CHAR,
    AST_STR,

    AST_VEC,
    AST_UNDEF,
} ASTBasicDataType_T;

typedef enum AST_EXPR_TYPE
{   
    AST_EX_CONSTANT,
    AST_EX_CLOSURE,
    AST_EX_OP,
    AST_EX_NIL,
    AST_EX_BOOL_OP,
    AST_EX_NOT,
    AST_EX_NEGATE,
    AST_EX_VAR_CALL,
    AST_EX_FN_CALL,
    AST_EX_ARRAY,
} ASTExprType_T;

typedef enum AST_EXPR_OP_TYPE
{
    AST_OP_ADD,
    AST_OP_SUB,
    AST_OP_MULT,
    AST_OP_DIV,
} ASTExprOpType_T;

typedef enum AST_EXPR_BOOL_OP_TYPE
{
    AST_OP_GT,  //greater
    AST_OP_LT,  //less
    AST_OP_EQ,  //equal
    AST_OP_GE,  //greater equal
    AST_OP_LE,  //less equal
    AST_OP_NE,  //not equal
    AST_OP_OR,  //or
    AST_OP_AND, //and
} ASTExprBoolOpType_T;

typedef enum AST_COMPOUND_INSTRUCTION_TYPE
{
    AST_CI_ASSIGN,
    AST_CI_LOCALDEF,
    AST_CI_FN_CALL,
    AST_CI_IF_STMT,
    AST_CI_FOR_STMT,
    AST_CI_WHILE_STMT,
    AST_CI_EXIT_STMT,
    AST_CI_RETURN_STMT,
} ASTCompoundInstructionType_T;

typedef struct AST_ROOT_STRUCT
{
    //TODO: list_T* customDataTypes;
    list_T* globals;
    list_T* functions;
} ASTRoot_T;

typedef struct AST_DATA_TYPE ASTDataType_T;

struct AST_DATA_TYPE
{
    ASTBasicDataType_T basicType;
    ASTDataType_T* innerType;
};

typedef struct AST_EXPR_STRUCT
{
    ASTExprType_T type;
    void* expr;
} ASTExpr_T;

typedef struct AST_EXPR_CONSTANT_STRUCT
{
    ASTDataType_T* dataType;
    void* value;
} ASTExprConstant_T;

typedef struct AST_EXPR_CLOSURE_STRUCT
{
    ASTDataType_T* dataType;
    ASTExpr_T* value;
} ASTExprClosure_T;

typedef struct AST_EXPR_OP_STRUCT
{
    ASTExprOpType_T type;
    ASTDataType_T* dataType;
    ASTExpr_T* left;
    ASTExpr_T* right;
} ASTExprOp_T;

typedef struct AST_EXPR_BOOL_OP_STRUCT
{
    ASTExprBoolOpType_T type;
    ASTDataType_T* dataType;
    ASTExpr_T* left;
    ASTExpr_T* right;
} ASTExprBoolOp_T;

typedef struct AST_EXPR_ARRAY_STRUCT
{
    ASTDataType_T* dataType;
    list_T* indices;
} ASTExprArray_T;

typedef struct AST_EXPR_NOT_STRUCT
{
    ASTDataType_T* dataType;
    ASTExpr_T* value;
} ASTExprNot_T;

typedef struct AST_EXPR_NEGATE_STRUCT
{
    ASTDataType_T* dataType;
    ASTExpr_T* value;
} ASTExprNegate_T;

typedef struct AST_EXPR_VAR_CALL_STRUCT
{
    ASTDataType_T* dataType;
    char* callee;
} ASTExprVarCall_T;

typedef struct AST_EXPR_FN_CALL_STRUCT
{
    ASTDataType_T* dataType;
    char* callee;
    list_T* args;
} ASTExprFnCall_T;

typedef struct AST_ASSIGNMENT_STRUCT
{
    ASTDataType_T* dataType;
    ASTExpr_T* value;
    ASTExprVarCall_T* target;
} ASTAssignment_T;

typedef struct AST_GLOBAL_STRUCT
{
    char* name;
    ASTDataType_T* dataType;
    ASTExpr_T* value;
} ASTGlobal_T;

typedef struct AST_LOCAL_STRUCT
{
    char* name;
    ASTDataType_T* dataType;
    ASTExpr_T* value;
} ASTLocal_T;

typedef struct AST_COMPOUND_STRUCT
{
    list_T* locals;
    list_T* body;
} ASTCompound_T;

typedef struct AST_COMPOUND_INSTRUCTION_STRUCT
{
    ASTCompoundInstructionType_T type;
    void* ptr;
} ASTCompoundInstruction_T;

typedef struct AST_FUNCTION_STRUCT
{
    char* name;
    ASTDataType_T* returnType;
    list_T* args;

    ASTCompound_T* body;
} ASTFunction_T;

typedef struct AST_ARGUMENT_STRUCT
{
    char* name;
    ASTDataType_T* dataType;
} ASTArgument_T;

typedef struct AST_IF_STMT_STRUCT
{
    ASTExpr_T* condition;
    ASTCompound_T* ifBody;
    ASTCompound_T* elseBody;
} ASTIfStmt_T;

typedef struct AST_WHILE_STMT_STRUCT
{
    ASTExpr_T* condition;
    ASTCompound_T* body;
} ASTWhileStmt_T;

typedef struct AST_FOR_STMT_STRUCT
{
    ASTExpr_T* condition;
    ASTAssignment_T* increment;
    ASTLocal_T* counterVar;
    ASTCompound_T* body;
} ASTForStmt_T;

typedef struct AST_RETURN_STMT_STRUCT
{
    ASTExpr_T* returnValue;
} ASTReturnStmt_T;

typedef struct AST_EXIT_STMT_STRUCT
{
    ASTExpr_T* exitCode;
} ASTExitStmt_T;

ASTRoot_T* initASTRoot();

ASTGlobal_T* initASTGlobal(char* name, ASTDataType_T* dataType, ASTExpr_T* value);
ASTArgument_T* initASTArgument(char* name, ASTDataType_T* dataType);
ASTDataType_T* initASTDataType_T(ASTBasicDataType_T dataType);
ASTFunction_T* initASTFunction(char* name);
ASTLocal_T* initASTLocal(char* name, ASTDataType_T* dataType, ASTExpr_T* value);

ASTExprFnCall_T* initASTFunctionCall(char* callee);
ASTExprVarCall_T* initASTVarCall(char* callee);

ASTAssignment_T* initASTAssingment(ASTExprVarCall_T* target, ASTExpr_T* value);

ASTExpr_T* initASTExpr(ASTExprType_T type, void* value);
ASTExpr_T* initASTExprNil();
ASTExprConstant_T* initASTConstant(ASTDataType_T* dataType, void* ptr);

ASTExprNegate_T* initASTNegate(ASTExpr_T* expr);
ASTExprNot_T* initASTNot(ASTExpr_T* expr);
ASTExprOp_T* initASTOp(ASTExprOpType_T type, ASTExpr_T* left, ASTExpr_T* right);
ASTExprBoolOp_T* initASTBoolOp(ASTExprBoolOpType_T type, ASTExpr_T* left, ASTExpr_T* right);

ASTExprClosure_T* initASTClosure(ASTExpr_T* value);
ASTExprArray_T* initASTArrayExpr();

ASTExitStmt_T* initASTExitStmt(ASTExpr_T* exitCode);
ASTReturnStmt_T* initASTReturnStmt(ASTExpr_T* returnValue);
ASTIfStmt_T* initASTIfStmt(ASTExpr_T* condition);
ASTWhileStmt_T* initASTWhileStmt(ASTExpr_T* condition);
ASTForStmt_T* initASTForStmt(ASTLocal_T* counterVar, ASTExpr_T* condition, ASTAssignment_T* increment);

ASTCompound_T* initASTCompound();
ASTCompoundInstruction_T* initCompoundInstruction(ASTCompoundInstructionType_T type, void* ptr);

#endif