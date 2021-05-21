#ifndef CSPYDR_AST_H
#define CSPYDR_AST_H

#include "stdbool.h"

#include "../list.h"

typedef enum 
{
    AST_I32,
    AST_I64,
    AST_U32,
    AST_U64,
    AST_F32,
    AST_F64,
    AST_BOOL,
    AST_CHAR,
    AST_ARRAY,
    AST_STRUCT,
    AST_TYPEDEF,    // this is the default type for the parser and will later be replaced with the typedef'd type
    AST_ENUM,
    AST_POINTER,
    AST_VOID,
} ASTDataType_T;

typedef enum 
{
    STMT_LOOP,
    STMT_IF,
    STMT_RETURN,
    STMT_LET,
    STMT_EXPRESSION,
    STMT_MATCH
} ASTStmtType_T;

typedef enum
{
    EXPR_PREFIX,
    EXPR_INFIX,
    EXPR_POSTFIX,
    EXPR_INDEX,
    EXPR_CALL,
    EXPR_NIL,
    EXPR_IDENTIFIER,
    EXPR_INT_LITERAL,
    EXPR_BOOL_LITERAL,
    EXPR_FLOAT_LITERAL,
    EXPR_CHAR_LITERAL,
    EXPR_STRING_LITERAL,
    EXPR_ARRAY_LITERAL,
    EXPR_STRUCT_LITERAL,
} ASTExprType_T;

typedef enum
{
    OP_ADD,
    OP_SUB,
    OP_MULT,
    OP_DIV,
    OP_GT,
    OP_LT,
    OP_EQ,
    OP_GT_EQ,
    OP_LT_EQ,
    OP_NOT_EQ,
    OP_ASSIGN,
    OP_RANGE,
} ASTInfixOpType_T;

typedef enum 
{
    OP_NOT,
    OP_NEGATE,
    OP_BIT_NEG,
    OP_REF,
    OP_DEREF,
} ASTPrefixOpType_T;

typedef enum
{
    OP_INC,
    OP_DEC,
} ASTPostfixOpType_T;

typedef struct AST_TYPE_STRUCT
{
    ASTDataType_T type;
    struct AST_TYPE_STRUCT* subtype;
    char* callee;   // callee for typedefs
    void* body;     // body for enums and structs

    bool free;
    unsigned int line;
    unsigned int pos;
} ASTType_T;

ASTType_T* initASTType(ASTDataType_T type, ASTType_T* subtype, void* body, char* callee, unsigned int line, unsigned int pos);
void freeASTType(ASTType_T* t);

typedef struct AST_STRUCT_TYPE_STRUCT
{
    list_T* fieldTypes;
    list_T* fieldNames;
} ASTStructType_T;

ASTStructType_T* initASTStructType(list_T* fieldTypes, list_T* fieldNames);
void freeASTStructType(ASTStructType_T* s);

typedef struct AST_ENUM_TYPE_STRUCT
{
    list_T* fields;
} ASTEnumType_T;

ASTEnumType_T* initASTEnumType(list_T* fields);
void freeASTEnumType(ASTEnumType_T* e);

typedef struct AST_COMPOUND_STRUCT
{
    list_T* stmts;
} ASTCompound_T;

ASTCompound_T* initASTCompound(list_T* stmts);
void freeASTCompound(ASTCompound_T* c);

/////////////////////////////////
// Expressions                 //
/////////////////////////////////

typedef struct AST_EXPRESSION_STRUCT
{
    ASTType_T* dataType;
    ASTExprType_T type;
    void* expr;
} ASTExpr_T;

ASTExpr_T* initASTExpr(ASTType_T* dataType, ASTExprType_T type, void* ptr);
void freeASTExpr(ASTExpr_T* e);

typedef struct AST_INFIX_STRUCT
{
    ASTInfixOpType_T op;

    ASTExpr_T* left;
    ASTExpr_T* right;
} ASTInfix_T;

ASTInfix_T* initASTInfix(ASTInfixOpType_T op, ASTExpr_T* right, ASTExpr_T* left);
void freeASTInfix(ASTInfix_T* i);

typedef struct AST_PREFIX_STRUCT
{
    ASTPrefixOpType_T op;

    ASTExpr_T* right;
} ASTPrefix_T;

ASTPrefix_T* initASTPrefix(ASTPrefixOpType_T op, ASTExpr_T* right);
void freeASTPrefix(ASTPrefix_T* p);

typedef struct AST_POSTFIX_STRUCT
{
    ASTPostfixOpType_T op;
    
    ASTExpr_T* left;
} ASTPostfix_T;

ASTPostfix_T* initASTPostfix(ASTPostfixOpType_T op, ASTExpr_T* left);
void freeASTPostfix(ASTPostfix_T* p);

typedef struct AST_IDENTIFIER_STRUCT
{
    char* callee;
    struct AST_IDENTIFIER_STRUCT* childId;
    bool isPtr;
} ASTIdentifier_T;

ASTIdentifier_T* initASTIdentifier(const char* callee, ASTIdentifier_T* child);
void freeASTIdentifier(ASTIdentifier_T* i);

typedef struct AST_CALL_STRUCT
{
    list_T* args;
    char* callee;
} ASTCall_T;

ASTCall_T* initASTCall(const char* callee, list_T* args);
void freeASTCall(ASTCall_T* c);

typedef struct AST_INDEX_STRUCT
{
    ASTExpr_T* idx;
    ASTExpr_T* value;
} ASTIndex_T;

ASTIndex_T* initASTIndex(ASTExpr_T* value, ASTExpr_T* idx);
void freeASTIndex(ASTIndex_T* i);

/////////////////////////////////
// Literals                    //
/////////////////////////////////

typedef struct AST_INT_LITERAL
{
    int _int;
} ASTInt_T;

ASTInt_T* initASTInt(int _int);
void freeASTInt(ASTInt_T* i);

typedef struct AST_FLOAT_LITERAL
{
    float _float;
} ASTFloat_T;

ASTFloat_T* initASTFloat(float _float);
void freeASTFloat(ASTFloat_T* f);

typedef struct AST_CHAR_LITERAL
{
    char _char;
} ASTChar_T;

ASTChar_T* initASTChar(char _char);
void freeASTChar(ASTChar_T* c);

typedef struct AST_STRING_LITERAL
{
    char* _string;
    unsigned int len;
} ASTString_T;

ASTString_T* initASTString(const char* _string);
void freeASTString(ASTString_T* s);

typedef struct AST_BOOL_LITERAL
{   
    bool _bool;
} ASTBool_T;

ASTBool_T* initASTBool(bool _bool);
void freeASTBool(ASTBool_T* b);

typedef struct AST_NIL_LITERAL
{
} ASTNil_T;

ASTNil_T* initASTNil();
void freeASTNil(ASTNil_T* n);

typedef struct AST_ARRAY_LITERAL
{
    list_T* exprs;
    unsigned int len;
} ASTArray_T;

ASTArray_T* initASTArray(list_T* exprs);
void freeASTArray(ASTArray_T* a);

typedef struct AST_STRUCT_LITERAL
{
    list_T* exprs;
    list_T* fields;
} ASTStruct_T;

ASTStruct_T* initASTStruct(list_T* exprs, list_T* fields);
void freeASTStruct(ASTStruct_T* s);

/////////////////////////////////
// Statements                  //
/////////////////////////////////

typedef struct AST_STATEMENT_STRUCT
{  
    ASTStmtType_T type;
    void* stmt;
} ASTStmt_T;

ASTStmt_T* initASTStmt(ASTStmtType_T type, void* stmt);
void freeASTStmt(ASTStmt_T* s);

typedef struct AST_RETURN_STRUCT
{
    ASTExpr_T* value;
} ASTReturn_T;

ASTReturn_T* initASTReturn(ASTExpr_T* value);
void freeASTReturn(ASTReturn_T* r);

typedef struct AST_If_STRUCT
{
    ASTExpr_T* condition;
    ASTCompound_T* ifBody;
    ASTCompound_T* elseBody;   
} ASTIf_T;

ASTIf_T* initASTIf(ASTExpr_T* condition, ASTCompound_T* ifBody, ASTCompound_T* elseBody);
void freeASTIf(ASTIf_T* i);

typedef struct AST_LOOP_STRUCT
{
    ASTExpr_T* condition;

    ASTCompound_T* body;
} ASTLoop_T;

ASTLoop_T* initASTLoop(ASTExpr_T* condition, ASTCompound_T* body);
void freeASTLoop(ASTLoop_T* l);

typedef struct AST_LOCAL_STRUCT
{
    ASTType_T* dataType;
    ASTExpr_T* value;
    char* name;

    bool mutable;

    unsigned int line;
    unsigned int pos;
} ASTLocal_T;

ASTLocal_T* initASTLocal(ASTType_T* dataType, ASTExpr_T* value, const char* name, unsigned int line, unsigned int pos);
void freeASTLocal(ASTLocal_T* l);

typedef struct AST_MATCH_STRUCT
{
    list_T* cases;
    list_T* bodys;
    ASTExpr_T* condition;
    ASTCompound_T* defaultBody;
} ASTMatch_T;

ASTMatch_T* initASTMatch(ASTExpr_T* condition, list_T* cases, list_T* bodys, ASTCompound_T* defaultBody);
void freeASTMatch(ASTMatch_T* m);

typedef struct AST_EXPR_STMT_STRUCT
{
    ASTExpr_T* expr;
} ASTExprStmt_T;

ASTExprStmt_T* initASTExprStmt(ASTExpr_T* expr);
void freeASTExprStmt(ASTExprStmt_T* e);

/////////////////////////////////
// base structures             //
/////////////////////////////////

typedef struct AST_ARGUMENT_STRUCT
{
    char* name;
    ASTType_T* dataType;
} ASTArgument_T;

ASTArgument_T* initASTArgument(const char* name, ASTType_T* dataType);
void freeASTArgument(ASTArgument_T* a);

typedef struct AST_FUCTION_STRUCT
{
    char* name;
    ASTCompound_T* body;
    ASTType_T* returnType;
    list_T* args;

    unsigned int line;
    unsigned int pos;
} ASTFunction_T;

ASTFunction_T* initASTFunction(const char* name, ASTType_T* returnType, ASTCompound_T* body, list_T* args, unsigned int line, unsigned int pos);
void freeASTFunction(ASTFunction_T* f);

typedef struct AST_GLOBAL_STRUCT
{
    char* name;
    ASTType_T* type;
    ASTExpr_T* value;

    bool mutable;

    unsigned int line;
    unsigned int pos;
} ASTGlobal_T;

ASTGlobal_T* initASTGlobal(const char* name, ASTType_T* type, ASTExpr_T* value, unsigned int line, unsigned int pos);
void freeASTGlobal(ASTGlobal_T* g);

typedef struct AST_TYPEDEF_STRUCT
{
    ASTType_T* dataType;
    char* name;

    unsigned int line;
    unsigned int pos;
} ASTTypedef_T;

ASTTypedef_T* initASTTypedef(ASTType_T* type, const char* name, unsigned int line, unsigned int pos);
void freeASTTypedef(ASTTypedef_T* t);

typedef struct AST_FILE_STRUCT
{
    char* filepath;

    list_T* types;
    list_T* globals;
    list_T* functions;
} ASTFile_T;

ASTFile_T* initASTFile(const char* filepath);
void freeASTFile(ASTFile_T* f);

typedef struct AST_PROGRAM_STRUCT
{
    char* mainFile;

    list_T* files;
} ASTProgram_T;

ASTProgram_T* initASTProgram(const char* mainFile);
void freeASTProgram(ASTProgram_T* p);

#endif