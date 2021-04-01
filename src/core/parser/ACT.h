#ifndef CSPYDR_ACTION_TREE_H
#define CSPYDR_ACTION_TREE_H

#include "../list.h"
#include "AST.h"

typedef enum ACT_BASIC_DATA_TYPE
{
    ACT_I8,
    ACT_I16,
    ACT_I32,
    ACT_I64,

    ACT_U8,
    ACT_U16,
    ACT_U32,
    ACT_U64,

    ACT_F32,
    ACT_F64,

    ACT_VOID,
    ACT_BOOL,
    ACT_CHAR,
    ACT_STR,

    ACT_VEC,

    ACT_NIL,
    ACT_UNDEF,
} ACTBasicDataType_T;

typedef enum ACT_EXPR_TYPE
{   
    ACT_CONSTANT,
    ACT_CLOSURE,
    ACT_OP,
    ACT_BOOL_OP,
    ACT_NOT,
    ACT_NEGATE,
    ACT_VAR_CALL,
    ACT_FN_CALL,
} ACTExprType_T;

typedef enum ACT_EXPR_OP_TYPE
{
    ACT_OP_ADD,
    ACT_OP_SUB,
    ACT_OP_MULT,
    ACT_OP_DIV,
} ACTExprOpType_T;

typedef enum ACT_EXPR_BOOL_OP_TYPE
{
    ACT_OP_GT,  //greater
    ACT_OP_LT,  //less
    ACT_OP_EQ,  //equal
    ACT_OP_GE,  //greater equal
    ACT_OP_LE,  //less equal
    ACT_OP_NE,  //not equal
} ACTExprBoolOpType_T;

typedef struct ACT_ROOT_STRUCT
{
    //TODO: list_T* customDataTypes;
    list_T* globals;
    list_T* functions;
} ACTRoot_T;

typedef struct ACT_DATA_TYPE ACTDataType_T;

struct ACT_DATA_TYPE
{
    ACTBasicDataType_T basicType;
    ACTDataType_T* innerType;
};

typedef struct ACT_EXPR_STRUCT
{
    ACTExprType_T type;
    void* expr;
} ACTExpr_T;

typedef struct ACT_EXPR_CONSTANT_STRUCT
{
    ACTDataType_T* dataType;
    void* value;
} ACTExprConstant_T;

typedef struct ACT_EXPR_CLOSURE_STRUCT
{
    ACTDataType_T* dataType;
    ACTExpr_T* value;
} ACTExprClosure_T;

typedef struct ACT_EXPR_OP_STRUCT
{
    ACTDataType_T* dataType;
    ACTExpr_T* left;
    ACTExpr_T* right;
} ACTExprOp_T;

typedef struct ACT_EXPR_BOOL_OP_STRUCT
{
    ACTDataType_T* dataType;
    ACTExpr_T* left;
    ACTExpr_T* right;
} ACTExprBoolOp_T;

typedef struct ACT_EXPR_NOT_STRUCT
{
    ACTDataType_T* dataType;
    ACTExpr_T* value;
} ACTExprNot_T;

typedef struct ACT_EXPR_NEGATE_STRUCT
{
    ACTDataType_T* dataType;
    ACTExpr_T* value;
} ACTExprNegate_T;

typedef struct ACT_EXPR_VAR_CALL_STRUCT
{
    ACTDataType_T* dataType;
    char* callee;
} ACTExprVarCall_T;

typedef struct ACT_EXPR_FN_CALL_STRUCT
{
    ACTDataType_T* dataType;
    char* callee;
    list_T* args;
} ACTExprFnCall_T;

typedef struct ACT_ASSIGNMENT_STRUCT
{
    ACTDataType_T* dataType;
    ACTExpr_T* value;
    ACTExprVarCall_T* target;
} ACTAssignment_T;

typedef struct ACT_GLOBAL_STRUCT
{
    char* name;
    ACTDataType_T* dataType;
} ACTGlobal_T;

typedef struct ACT_LOCAL_STRUCT
{
    char* name;
    ACTDataType_T* dataType;
} ACTLocal_T;

typedef struct ACT_COMPOUND_STRUCT
{
    list_T* locals;
    list_T* body;
} ACTCompound_T;

typedef struct ACT_FUNCTION_STRUCT
{
    char* name;
    ACTDataType_T* returnType;
    list_T* args;

    ACTCompound_T* body;
} ACTFunction_T;

typedef struct ACT_IF_STMT_STRUCT
{
    ACTExpr_T* condition;
    ACTCompound_T* ifBody;
    ACTCompound_T* elseBody;
} ACTIfStmt_T;

typedef struct ACT_WHILE_STMT_STRUCT
{
    ACTExpr_T* condition;
    ACTCompound_T* body;
} ACTWhileStmt_T;

typedef struct ACT_FOR_STMT_STRUCT
{
    ACTExpr_T* condition;
    ACTAssignment_T* increment;
    ACTLocal_T* counterVar;
} ACTForStmt_T;

typedef struct ACT_RETURN_STMT_STRUCT
{
    ACTExpr_T* returnValue;
} ACTReturnStmt_T;

typedef struct ACT_EXIT_STMT_STRUCT
{
    ACTExpr_T* exitCode;
} ACTExitStmt_T;

ACTRoot_T* initACTRoot();

ACTFunction_T* initACTFunction(char* name, AST_T* returnType);
ACTDataType_T* initACTDataType(AST_T* ast);
ACTGlobal_T* initACTGlobal(char* name, AST_T* dataType);

void registerFunction(ACTRoot_T* root, AST_T* func);
void registerGlobal(ACTRoot_T* root, AST_T* global);

#endif