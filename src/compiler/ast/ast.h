#ifndef CSPYDR_AST_H
#define CSPYDR_AST_H

#include "stdbool.h"

#include "../list.h"

typedef enum 
{
    AST_I8,
    AST_I16,
    AST_I32,
    AST_I64,

    AST_U8,
    AST_U16,
    AST_U32,
    AST_U64,

    AST_F32,
    AST_F64,
    AST_F80,

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

    int size;

    bool free;
    bool is_primitive;
    unsigned int line;
    unsigned int pos;
} ASTType_T;

ASTType_T* init_ast_type(ASTDataType_T type, ASTType_T* subtype, void* body, char* callee, unsigned int line, unsigned int pos);
void free_ast_type(ASTType_T* t);

typedef struct AST_STRUCT_TYPE_STRUCT
{
    List_T* field_types;
    List_T* field_names;
} ASTStructType_T;

ASTStructType_T* init_ast_struct_type(List_T* field_types, List_T* field_names);
void free_ast_struct_type(ASTStructType_T* s);

typedef struct AST_ENUM_TYPE_STRUCT
{
    List_T* fields;
} ASTEnumType_T;

ASTEnumType_T* init_ast_enum_type(List_T* fields);
void free_ast_enum_type(ASTEnumType_T* e);

typedef struct AST_COMPOUND_STRUCT
{
    List_T* stmts;
} ASTCompound_T;

ASTCompound_T* init_ast_compound(List_T* stmts);
void free_ast_compound(ASTCompound_T* c);

/////////////////////////////////
// Expressions                 //
/////////////////////////////////

typedef struct AST_EXPRESSION_STRUCT
{
    ASTType_T* data_type;
    ASTExprType_T type;
    void* expr;
} ASTExpr_T;

ASTExpr_T* init_ast_expr(ASTType_T* data_type, ASTExprType_T type, void* ptr);
void free_ast_expr(ASTExpr_T* e);

typedef struct AST_INFIX_STRUCT
{
    ASTInfixOpType_T op;

    ASTExpr_T* left;
    ASTExpr_T* right;
} ASTInfix_T;

ASTInfix_T* init_ast_infix(ASTInfixOpType_T op, ASTExpr_T* right, ASTExpr_T* left);
void free_ast_infix(ASTInfix_T* i);

typedef struct AST_PREFIX_STRUCT
{
    ASTPrefixOpType_T op;

    ASTExpr_T* right;
} ASTPrefix_T;

ASTPrefix_T* init_ast_prefix(ASTPrefixOpType_T op, ASTExpr_T* right);
void free_ast_prefix(ASTPrefix_T* p);

typedef struct AST_POSTFIX_STRUCT
{
    ASTPostfixOpType_T op;
    
    ASTExpr_T* left;
} ASTPostfix_T;

ASTPostfix_T* init_ast_postfix(ASTPostfixOpType_T op, ASTExpr_T* left);
void free_ast_postfix(ASTPostfix_T* p);

typedef struct AST_IDENTIFIER_STRUCT
{
    char* callee;
    struct AST_IDENTIFIER_STRUCT* child_id;
    bool isPtr;
} ASTIdentifier_T;

ASTIdentifier_T* init_ast_identifier(const char* callee, ASTIdentifier_T* child);
void free_ast_identifier(ASTIdentifier_T* i);

typedef struct AST_CALL_STRUCT
{
    List_T* args;
    char* callee;
} ASTCall_T;

ASTCall_T* init_ast_call(const char* callee, List_T* args);
void free_ast_call(ASTCall_T* c);

typedef struct AST_INDEX_STRUCT
{
    ASTExpr_T* idx;
    ASTExpr_T* value;
} ASTIndex_T;

ASTIndex_T* init_ast_index(ASTExpr_T* value, ASTExpr_T* idx);
void free_ast_index(ASTIndex_T* i);

/////////////////////////////////
// Literals                    //
/////////////////////////////////

typedef struct AST_INT_LITERAL
{
    int _int;
} ASTInt_T;

ASTInt_T* init_ast_int(int _int);
void free_ast_int(ASTInt_T* i);

typedef struct AST_FLOAT_LITERAL
{
    float _float;
} ASTFloat_T;

ASTFloat_T* init_ast_float(float _float);
void free_ast_float(ASTFloat_T* f);

typedef struct AST_CHAR_LITERAL
{
    char _char;
} ASTChar_T;

ASTChar_T* init_ast_char(char _char);
void free_ast_char(ASTChar_T* c);

typedef struct AST_STRING_LITERAL
{
    char* _string;
    unsigned int len;
} ASTString_T;

ASTString_T* init_ast_string(const char* _string);
void free_ast_string(ASTString_T* s);

typedef struct AST_BOOL_LITERAL
{   
    bool _bool;
} ASTBool_T;

ASTBool_T* init_ast_bool(bool _bool);
void free_ast_bool(ASTBool_T* b);

typedef struct AST_NIL_LITERAL
{
} ASTNil_T;

ASTNil_T* init_ast_nil();
void free_ast_nil(ASTNil_T* n);

typedef struct AST_ARRAY_LITERAL
{
    List_T* exprs;
    unsigned int len;
} ASTArray_T;

ASTArray_T* init_ast_array(List_T* exprs);
void free_ast_array(ASTArray_T* a);

typedef struct AST_STRUCT_LITERAL
{
    List_T* exprs;
    List_T* fields;
} ASTStruct_T;

ASTStruct_T* init_ast_struct(List_T* exprs, List_T* fields);
void free_ast_struct(ASTStruct_T* s);

/////////////////////////////////
// Statements                  //
/////////////////////////////////

typedef struct AST_STATEMENT_STRUCT
{  
    ASTStmtType_T type;
    void* stmt;
} ASTStmt_T;

ASTStmt_T* init_ast_stmt(ASTStmtType_T type, void* stmt);
void free_ast_stmt(ASTStmt_T* s);

typedef struct AST_RETURN_STRUCT
{
    ASTExpr_T* value;
} ASTReturn_T;

ASTReturn_T* init_ast_return(ASTExpr_T* value);
void free_ast_return(ASTReturn_T* r);

typedef struct AST_If_STRUCT
{
    ASTExpr_T* condition;
    ASTCompound_T* if_body;
    ASTCompound_T* else_body;   
} ASTIf_T;

ASTIf_T* init_ast_if(ASTExpr_T* condition, ASTCompound_T* if_body, ASTCompound_T* else_body);
void free_ast_if(ASTIf_T* i);

typedef struct AST_LOOP_STRUCT
{
    ASTExpr_T* condition;

    ASTCompound_T* body;
} ASTLoop_T;

ASTLoop_T* init_ast_loop(ASTExpr_T* condition, ASTCompound_T* body);
void free_ast_loop(ASTLoop_T* l);

typedef struct AST_LOCAL_STRUCT
{
    ASTType_T* type;
    ASTExpr_T* value;
    char* name;

    bool isMutable;

    unsigned int line;
    unsigned int pos;
} ASTLocal_T;

ASTLocal_T* init_ast_local(ASTType_T* dataType, ASTExpr_T* value, const char* name, unsigned int line, unsigned int pos);
void free_ast_local(ASTLocal_T* l);

typedef struct AST_MATCH_STRUCT
{
    List_T* cases;
    List_T* bodys;
    ASTExpr_T* condition;
    ASTCompound_T* default_case;
} ASTMatch_T;

ASTMatch_T* init_ast_match(ASTExpr_T* condition, List_T* cases, List_T* bodys, ASTCompound_T* default_case);
void free_ast_match(ASTMatch_T* m);

typedef struct AST_EXPR_STMT_STRUCT
{
    ASTExpr_T* expr;
} ASTExprStmt_T;

ASTExprStmt_T* init_ast_expr_stmt(ASTExpr_T* expr);
void free_ast_expr_stmt(ASTExprStmt_T* e);

/////////////////////////////////
// base structures             //
/////////////////////////////////

typedef struct AST_ARGUMENT_STRUCT
{
    char* name;
    ASTType_T* type;
    int offset;
} ASTArgument_T;

ASTArgument_T* init_ast_argument(const char* name, ASTType_T* data_type);
void free_ast_argument(ASTArgument_T* a);

typedef struct AST_FUCTION_STRUCT
{
    char* name;
    ASTCompound_T* body;
    ASTType_T* return_type;
    List_T* args;

    unsigned int line;
    unsigned int pos;
} ASTFunction_T;

ASTFunction_T* init_ast_function(const char* name, ASTType_T* return_type, ASTCompound_T* body, List_T* args, unsigned int line, unsigned int pos);
void free_ast_function(ASTFunction_T* f);

typedef struct AST_GLOBAL_STRUCT
{
    char* name;
    ASTType_T* type;
    ASTExpr_T* value;

    bool is_mutable;

    unsigned int line;
    unsigned int pos;
} ASTGlobal_T;

ASTGlobal_T* init_ast_global(const char* name, ASTType_T* type, ASTExpr_T* value, unsigned int line, unsigned int pos);
void free_ast_global(ASTGlobal_T* g);

typedef struct AST_TYPEDEF_STRUCT
{
    ASTType_T* data_type;
    char* name;

    unsigned int line;
    unsigned int pos;
} ASTTypedef_T;

ASTTypedef_T* init_ast_typedef(ASTType_T* type, const char* name, unsigned int line, unsigned int pos);
void free_ast_typedef(ASTTypedef_T* t);

typedef struct AST_FILE_STRUCT
{
    char* filepath;

    List_T* types;
    List_T* globals;
    List_T* functions;
} ASTFile_T;

ASTFile_T* init_ast_file(const char* filepath);
void free_ast_file(ASTFile_T* f);

typedef struct AST_PROGRAM_STRUCT
{
    char* main_file;

    List_T* files;
} ASTProgram_T;

ASTProgram_T* init_ast_program(const char* main_file);
void free_ast_program(ASTProgram_T* p);

#endif