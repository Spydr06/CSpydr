#ifndef CSPYDR_AST_H
#define CSPYDR_AST_H

#include "stdbool.h"

#include "list.h"
#include "hashmap.h"
#include "lexer/token.h"
#include "config.h"
#include "../../api/include/cspydr.h"

#include <stdint.h>
#include <stdio.h>

#define ND_NOOP CSPYDR_ND_NOOP
#define ND_ID CSPYDR_ND_ID
#define ND_INT CSPYDR_ND_INT
#define ND_LONG CSPYDR_ND_LONG
#define ND_ULONG CSPYDR_ND_ULONG
#define ND_FLOAT CSPYDR_ND_FLOAT
#define ND_DOUBLE CSPYDR_ND_DOUBLE
#define ND_BOOL CSPYDR_ND_BOOL
#define ND_CHAR CSPYDR_ND_CHAR
#define ND_STR CSPYDR_ND_STR
#define ND_NIL CSPYDR_ND_NIL
#define ND_ARRAY CSPYDR_ND_ARRAY
#define ND_STRUCT CSPYDR_ND_STRUCT
#define ND_ADD CSPYDR_ND_ADD
#define ND_SUB CSPYDR_ND_SUB
#define ND_MUL CSPYDR_ND_MUL
#define ND_DIV CSPYDR_ND_DIV
#define ND_MOD CSPYDR_ND_MOD
#define ND_NEG CSPYDR_ND_NEG
#define ND_BIT_NEG CSPYDR_ND_BIT_NEG
#define ND_NOT CSPYDR_ND_NOT
#define ND_REF CSPYDR_ND_REF
#define ND_DEREF CSPYDR_ND_DEREF
#define ND_EQ CSPYDR_ND_EQ
#define ND_NE CSPYDR_ND_NE
#define ND_GT CSPYDR_ND_GT
#define ND_GE CSPYDR_ND_GE
#define ND_LT CSPYDR_ND_LT
#define ND_LE CSPYDR_ND_LE
#define ND_AND CSPYDR_ND_AND
#define ND_OR CSPYDR_ND_OR
#define ND_LSHIFT CSPYDR_ND_LSHIFT
#define ND_RSHIFT CSPYDR_ND_RSHIFT
#define ND_XOR CSPYDR_ND_XOR
#define ND_BIT_OR CSPYDR_ND_BIT_OR
#define ND_BIT_AND CSPYDR_ND_BIT_AND
#define ND_INC CSPYDR_ND_INC
#define ND_DEC CSPYDR_ND_DEC
#define ND_CLOSURE CSPYDR_ND_CLOSURE
#define ND_ASSIGN CSPYDR_ND_ASSIGN
#define ND_MEMBER CSPYDR_ND_MEMBER
#define ND_CALL CSPYDR_ND_CALL
#define ND_INDEX CSPYDR_ND_INDEX
#define ND_CAST CSPYDR_ND_CAST
#define ND_SIZEOF CSPYDR_ND_SIZEOF
#define ND_ALIGNOF CSPYDR_ND_ALIGNOF
#define ND_PIPE CSPYDR_ND_PIPE
#define ND_HOLE CSPYDR_ND_HOLE
#define ND_LAMBDA CSPYDR_ND_LAMBDA
#define ND_ELSE_EXPR CSPYDR_ND_ELSE_EXPR
#define ND_TYPE_EXPR CSPYDR_ND_TYPE_EXPR
#define ND_BLOCK CSPYDR_ND_BLOCK
#define ND_IF CSPYDR_ND_IF
#define ND_TERNARY CSPYDR_ND_TERNARY
#define ND_LOOP CSPYDR_ND_LOOP
#define ND_WHILE CSPYDR_ND_WHILE
#define ND_FOR CSPYDR_ND_FOR
#define ND_FOR_RANGE CSPYDR_ND_FOR_RANGE
#define ND_MATCH CSPYDR_ND_MATCH
#define ND_MATCH_TYPE CSPYDR_ND_MATCH_TYPE
#define ND_CASE CSPYDR_ND_CASE
#define ND_CASE_TYPE CSPYDR_ND_CASE_TYPE
#define ND_RETURN CSPYDR_ND_RETURN
#define ND_EXPR_STMT CSPYDR_ND_EXPR_STMT
#define ND_BREAK CSPYDR_ND_BREAK
#define ND_CONTINUE CSPYDR_ND_CONTINUE
#define ND_DO_UNLESS CSPYDR_ND_DO_UNLESS
#define ND_DO_WHILE CSPYDR_ND_DO_WHILE
#define ND_LEN CSPYDR_ND_LEN
#define ND_USING CSPYDR_ND_USING
#define ND_WITH CSPYDR_ND_WITH
#define ND_STRUCT_MEMBER CSPYDR_ND_STRUCT_MEMBER
#define ND_ASM CSPYDR_ND_ASM
#define ND_DEFER CSPYDR_ND_DEFER
#define ND_KIND_LEN CSPYDR_ND_KIND_LEN

#define TY_I8 CSPYDR_TY_I8
#define TY_I16 CSPYDR_TY_I16
#define TY_I32 CSPYDR_TY_I32
#define TY_I64 CSPYDR_TY_I64
#define TY_U8 CSPYDR_TY_U8
#define TY_U16 CSPYDR_TY_U16
#define TY_U32 CSPYDR_TY_U32
#define TY_U64 CSPYDR_TY_U64
#define TY_F32 CSPYDR_TY_F32
#define TY_F64 CSPYDR_TY_F64
#define TY_F80 CSPYDR_TY_F80
#define TY_BOOL CSPYDR_TY_BOOL
#define TY_VOID CSPYDR_TY_VOID
#define TY_CHAR CSPYDR_TY_CHAR
#define TY_PTR CSPYDR_TY_PTR
#define TY_ARRAY CSPYDR_TY_ARRAY
#define TY_VLA CSPYDR_TY_VLA
#define TY_C_ARRAY CSPYDR_TY_C_ARRAY
#define TY_STRUCT CSPYDR_TY_STRUCT
#define TY_ENUM CSPYDR_TY_ENUM
#define TY_FN CSPYDR_TY_FN
#define TY_UNDEF CSPYDR_TY_UNDEF
#define TY_TYPEOF CSPYDR_TY_TYPEOF
#define TY_TEMPLATE CSPYDR_TY_TEMPLATE
#define TY_KIND_LEN CSPYDR_TY_KIND_LEN

#define OBJ_GLOBAL CSPYDR_OBJ_GLOBAL
#define OBJ_LOCAL CSPYDR_OBJ_LOCAL
#define OBJ_FUNCTION CSPYDR_OBJ_FUNCTION
#define OBJ_FN_ARG CSPYDR_OBJ_FN_ARG
#define OBJ_TYPEDEF CSPYDR_OBJ_TYPEDEF
#define OBJ_NAMESPACE CSPYDR_OBJ_NAMESPACE
#define OBJ_ENUM_MEMBER CSPYDR_OBJ_ENUM_MEMBER
#define OBJ_LAMBDA CSPYDR_OBJ_LAMBDA
#define OBJ_KIND_LEN CSPYDR_OBJ_KIND_LEN

typedef enum MAIN_FUNCTION_KIND {
    MFK_NO_ARGS,
    MFK_ARGV_PTR,
    MFK_ARGC_ARGV_PTR,
    MFK_ARGS_ARRAY
} MainFunctionKind_T;

typedef enum CSPYDR_AST_NODE_KIND_ENUM ASTNodeKind_T;
typedef enum CSPYDR_AST_OBJ_KIND_ENUM  ASTObjKind_T;
typedef enum CSPYDR_AST_TYPE_KIND_ENUM ASTTypeKind_T;
typedef struct AST_NODE_STRUCT         ASTNode_T;
typedef struct AST_IDENTIFIER_STRUCT   ASTIdentifier_T;
typedef struct AST_TYPE_STRUCT         ASTType_T;
typedef struct AST_OBJ_STRUCT          ASTObj_T;

struct AST_NODE_STRUCT
{
    ASTNodeKind_T kind;
    Token_T* tok;

    ASTType_T* data_type;

    // id
    ASTIdentifier_T* id;

    // literals
    union {
        i32 int_val;
        i64 offset;
        i64 long_val;
        u64 ulong_val;
        f32 float_val;
        f64 double_val;
        bool bool_val;
        char* str_val;

        TokenType_T cmp_kind; // kind of type comparisons
    };

    union {
        // expressions
        struct {
            ASTNode_T* left;
            ASTNode_T* right;
        };

        // type comparisons
        struct {
            ASTType_T* l_type;
            ASTType_T* r_type;
        };
        
        union {
            ASTNode_T* condition; // condition for loop, match, case and if statements
            List_T* ids; // using statements, list of ASTIdentifier_Ts
        };
    };

    union {
        // if statement
        struct {
            ASTNode_T* if_branch;
            ASTNode_T* else_branch;
        };

        // loop statement
        struct {
            ASTNode_T* body;
            ASTNode_T* init_stmt;
        };
        
        List_T* stmts; // block statements, list of ASTNode_Ts
        List_T* exprs; // closures, list of ASTNode_Ts

        // references
        ASTObj_T* called_obj;
        ASTObj_T* referenced_obj;

        ASTNode_T* return_val; // return

        // match statement
        ASTNode_T* default_case;
    };

    union {
        // loop, block statement
        List_T* locals; // list of ASTObj_Ts

        // calls, array literals
        List_T* args;   // list of ASTNode_Ts

        // match statement
        List_T* cases; // list of ASTNode_Ts

        // with statement, single object
        ASTObj_T* obj;
    };

    union {
        // call
        ASTObj_T* return_buffer;

        // with statement
        ASTObj_T* exit_fn;

        // lambda
        ASTObj_T* return_ptr;
        
        // array literal
        ASTObj_T* buffer;
    };

    union {
        ASTNode_T* expr;
        ASTNode_T* call;
        
        // sizeof
        ASTType_T* the_type;

        // lambda
        ASTObj_T* stack_ptr;
    };

    union { 
        bool is_default_case : 1; // case
        bool pass_by_stack   : 1; // call
        bool from_back       : 1; // index
    };

    // assignment expression
    bool is_assigning    : 1;
    bool is_initializing : 1;
    bool result_ignored  : 1;

    // expression statement
    bool is_constant : 1;
    
} __attribute__((packed));

struct AST_IDENTIFIER_STRUCT
{
    Token_T* tok;
    ASTIdentifier_T* outer;
    char* callee;
    bool global_scope;
} __attribute__((packed));

struct AST_TYPE_STRUCT 
{
    ASTTypeKind_T kind;
    Token_T* tok;

    i32 size;
    i32 align;

    ASTType_T* base;
    ASTIdentifier_T* id;

    union {
        struct {
            bool is_primitive : 1;
            bool is_constant  : 1;
            bool is_fn        : 1;
            bool is_union     : 1;
            bool is_variadic  : 1;
            bool no_warnings  : 1;
            bool generated    : 1;
        };
        u8 flags;
    };

    union {
        // functions
        List_T* arg_types;  // list of ASTType_Ts
        // arrays
        ASTNode_T* num_indices_node;

        // enums, structs
        List_T* members;    // list of ASTNode_Ts
        ASTObj_T* referenced_obj;
    };

    u64 num_indices;
} __attribute__((packed));

struct AST_OBJ_STRUCT 
{
    ASTObjKind_T kind;
    Token_T* tok;

    ASTIdentifier_T* id;
    i32 offset;
    i32 stack_size;

    // variables
    union {
        struct {
            bool is_constant    : 1;
            bool is_extern      : 1;
            bool is_extern_c    : 1;
            bool referenced     : 1;
            bool is_entry_point : 1;
            bool no_return      : 1;
            bool ignore_unused  : 1;
            bool generated      : 1;
        };
        u8 flags;
    };

    ASTType_T* data_type;
    ASTNode_T* value;
    List_T* args;
    List_T* objs;
    List_T* deferred;

    // functions
    ASTType_T* return_type;
    ASTNode_T* body;
    ASTObj_T* alloca_size;
    ASTObj_T* alloca_bottom;
    ASTObj_T* va_area;
    ASTObj_T* return_ptr;
} __attribute__((packed));

typedef struct AST_EXIT_FN_HANDLE_STRUCT
{
    Token_T* tok;
    ASTObj_T* fn;
    ASTType_T* type;
} ASTExitFnHandle_T;

typedef struct AST_PROG_STRUCT
{
    const char* main_file_path;
    const char* target_binary;

    List_T* imports;
    List_T* tuple_structs;
    List_T* type_exit_fns;

    ASTObj_T* entry_point;

    List_T* objs;   // list of ASTObj_Ts

    MainFunctionKind_T mfk;
} ASTProg_T;

ASTNode_T* init_ast_node(ASTNodeKind_T kind, Token_T* tok);
ASTType_T* init_ast_type(ASTTypeKind_T kind, Token_T* tok);

ASTIdentifier_T* init_ast_identifier(Token_T* tok, char* callee);

ASTObj_T* init_ast_obj(ASTObjKind_T kind, Token_T* tok);

void init_ast_prog(ASTProg_T* prog, const char* main_file_path, const char* target_binary, List_T* imports);

const char* obj_kind_to_str(ASTObjKind_T kind);
const char* type_kind_to_str(ASTTypeKind_T kind);
char* ast_type_to_str(char* dest, ASTType_T* ty, size_t size);
char* ast_id_to_str(char* dest, ASTIdentifier_T* id, size_t size);

void merge_ast_progs(ASTProg_T* dest, ASTProg_T* src);

extern const ASTIdentifier_T empty_id;

#endif