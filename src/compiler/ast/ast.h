#ifndef CSPYDR_AST_H
#define CSPYDR_AST_H

#include "stdbool.h"

#include "../list.h"
#include "../lexer/token.h"
#include "../globals.h"

#include <stdint.h>
#include <stdio.h>

typedef struct AST_NODE_STRUCT ASTNode_T;
typedef struct AST_IDENTIFIER_STRUCT ASTIdentifier_T;
typedef struct AST_TYPE_STRUCT ASTType_T;
typedef struct AST_OBJ_STRUCT ASTObj_T;

typedef enum {
    ND_NOOP,

    // identifiers
    ND_ID,            // x

    // literals
    ND_INT,     // 0
    ND_LONG,
    ND_LLONG, 
    ND_FLOAT,   // 0.1
    ND_DOUBLE,
    ND_BOOL,    // true, false
    ND_CHAR,    // 'x'
    ND_STR,     // "..."
    ND_NIL,     // nil

    ND_ARRAY,   // [2, 4, ...]
    ND_STRUCT,  // {3, 4, ...}

    // operators
    ND_ADD,     // +
    ND_SUB,     // -
    ND_MUL,     // *
    ND_DIV,     // /
    ND_MOD,     // %

    ND_NEG,     // unary -
    ND_BIT_NEG, // unary ~
    ND_NOT,     // unary !
    ND_REF,     // unary &
    ND_DEREF,   // unary *

    ND_EQ,      // ==
    ND_NE,      // !=
    ND_GT,      // >
    ND_GE,      // >=
    ND_LT,      // <
    ND_LE,      // <=

    ND_AND, // &&
    ND_OR,  // ||

    ND_LSHIFT,  // <<
    ND_RSHIFT,  // >>
    ND_XOR,     // ^
    ND_BIT_OR,  // |
    ND_BIT_AND, // &

    ND_INC,     // ++
    ND_DEC,     // --

    ND_CLOSURE, // ( )
    ND_ASSIGN,  // x = y

    ND_MEMBER,  // x.y
    ND_CALL,    // x(y, z)
    ND_INDEX,   // x[y]
    ND_CAST,    // x:i32

    ND_SIZEOF,  // typeof x
    ND_TYPEOF,  // sizeof x

    // statements
    ND_BLOCK,   // {...}
    ND_IF,      // if x {}
    ND_LOOP,    // loop {}
    ND_WHILE,   // while x {}
    ND_FOR,     // for let i: i32 = 0; i < x; i++ {}
    ND_MATCH,   // match x {}
    ND_CASE,    // x => {} !!only in match statements!!
    ND_RETURN,  // ret x;
    ND_EXPR_STMT, // "executable" expressions
    ND_BREAK,     // break;
    ND_CONTINUE,  // continue;
    ND_ASM,       // asm "<assembly code>";
    ND_LEN,       // len x
    ND_VA_ARG,    // va_arg x: i32

    ND_LAMBDA,

    ND_ENUM_MEMBER,     // enum members
    ND_STRUCT_MEMBER,  // struct members

} ASTNodeKind_T;

typedef enum {
    TY_I8,
    TY_I16,
    TY_I32,
    TY_I64,

    TY_U8,
    TY_U16,
    TY_U32,
    TY_U64,

    TY_F32,
    TY_F64,
    TY_F80,

    TY_BOOL,
    TY_VOID,
    TY_CHAR,

    TY_PTR,
    TY_ARR,
    TY_STRUCT,
    TY_OPAQUE_STRUCT,
    TY_ENUM,

    TY_LAMBDA,
    TY_TUPLE,
    TY_TEMPLATE,

    TY_UNDEF
} ASTTypeKind_T;

typedef enum {
    OBJ_GLOBAL,
    OBJ_LOCAL,
    OBJ_FUNCTION,
    OBJ_FN_ARG,
    OBJ_TYPEDEF,
    OBJ_NAMESPACE
} ASTObjKind_T;

struct AST_NODE_STRUCT
{
    ASTNodeKind_T kind;
    Token_T* tok;

    ASTType_T* data_type;

    // id
    ASTIdentifier_T* id;

    // literals
    union {
        int int_val;
        long long_val;
        long long llong_val;
        float float_val;
        double double_val;
        bool bool_val;
        char* str_val;  // also used for chars
    };

    // op
    ASTNode_T* left;
    ASTNode_T* right;

    // block
    List_T* stmts;  // list of ASTNode_Ts
    List_T* locals; // list of ASTObj_Ts

    // condition for loop, match, case and if statements
    ASTNode_T* condition;

    // if
    ASTNode_T* if_branch;
    ASTNode_T* else_branch;

    // loop
    ASTNode_T* body;
    ASTObj_T* counter_var;
    ASTNode_T* init_stmt;

    // return
    ASTNode_T* return_val;

    // match
    List_T* cases;           // list of ASTNode_Ts
    ASTNode_T* default_case;

    // case
    bool is_default_case: 1;

    // expression statement
    bool is_constant: 1;
    ASTNode_T* expr;

    // calls, array literals
    List_T* args;   // list of ASTNode_Ts
    List_T* template_types;
} __attribute__((packed));

struct AST_IDENTIFIER_STRUCT
{
    ASTObjKind_T kind; // kind of the object, which the name is referring to
    Token_T* tok;

    char callee[__CSP_MAX_TOKEN_SIZE];
    ASTIdentifier_T* outer;
} __attribute((packed));

struct AST_TYPE_STRUCT 
{
    ASTTypeKind_T kind;
    Token_T* tok;

    ASTType_T* base;
    int size;

    ASTIdentifier_T* id;

    bool is_primitive: 1;
    bool is_constant: 1;
    bool is_complex: 1;
    bool is_volatile: 1;
    bool is_atomic: 1;
    bool is_fn: 1;

    // functions
    List_T* arg_types;  // list of ASTType_Ts

    // arrays
    ASTNode_T* num_indices;

    // enums, structs
    List_T* members;    // list of ASTNode_Ts
} __attribute__((packed));

struct AST_OBJ_STRUCT 
{
    ASTObjKind_T kind;
    Token_T* tok;

    ASTIdentifier_T* id;

    // variables
    bool is_constant : 1;
    bool is_extern : 1;

    ASTType_T* data_type;
    ASTNode_T* value;

    // functions
    ASTIdentifier_T* va_name;

    ASTType_T* return_type;
    List_T* args;           // list of ASTObj_Ts
    ASTNode_T* body;
    List_T* templates;

    // namespaces
    List_T* objs;
} __attribute__((packed));

typedef struct AST_PROG_STRUCT
{
    const char* main_file_path;
    const char* target_binary;

    List_T* imports;
    List_T* lambda_literals;
    List_T* tuple_structs;

    List_T* objs;   // list of ASTObj_Ts
} ASTProg_T;

ASTNode_T* init_ast_node(ASTNodeKind_T kind, Token_T* tok);
ASTType_T* init_ast_type(ASTTypeKind_T kind, Token_T* tok);

ASTIdentifier_T* init_ast_identifier(Token_T* tok, char callee[__CSP_MAX_TOKEN_SIZE]);

ASTObj_T* init_ast_obj(ASTObjKind_T kind, Token_T* tok);

void init_ast_prog(ASTProg_T* prog, const char* main_file_path, const char* target_binary, List_T* imports);
void free_ast_prog(ASTProg_T* prog);

const char* obj_kind_to_str(ASTObjKind_T kind);

void merge_ast_progs(ASTProg_T* dest, ASTProg_T* src);

#endif