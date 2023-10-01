#ifndef CSPYDR_AST_H
#define CSPYDR_AST_H

#include "stdbool.h"

#include "list.h"
#include "hashmap.h"
#include "lexer/token.h"

#define __CSPYDR_INTERNAL_USE
#include "../../api/include/cspydr.h"

#include <stdint.h>
#include <stdio.h>

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

typedef enum UNPACK_MODE {
    UMODE_NONE = 0b00,
    UMODE_FTOB = 0b01,
    UMODE_BTOF = 0b10
} UnpackMode_T;

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
            List_T* lambdas; // lambda expressions in block statements
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

        TokenType_T mode; // case mode
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

    UnpackMode_T unpack_mode : 2;

    // assignment expression
    bool is_assigning    : 1;
    bool output          : 1;
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
            bool no_return    : 1;
        };
        u8 flags;
    };

    union {
        // functions
        List_T* arg_types;  // list of ASTType_Ts
        // arrays
        ASTNode_T* num_indices_node;
        // interfaces
        List_T* func_decls; // list of ASTObj_Ts

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

    union {
        struct {
            bool is_constant    : 1;
            bool is_extern      : 1;
            bool is_extern_c    : 1;
            bool referenced     : 1;
            bool is_entry_point : 1;
            bool ignore_unused  : 1;
            bool generated      : 1;
            bool private        : 1;
            bool deprecated     : 1;
            bool after_main     : 1;
            bool before_main    : 1;
            u8 __unused__       : 5;
        };
        u16 flags;
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
    const char* exported;
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

    List_T* files;
    List_T* tokens;

    List_T* tuple_structs;
    List_T* type_exit_fns;

    ASTObj_T* entry_point;

    List_T* objs;   // list of ASTObj_Ts

    List_T* before_main;
    List_T* after_main;

    MainFunctionKind_T mfk;
} ASTProg_T;

ASTNode_T* init_ast_node(ASTNodeKind_T kind, Token_T* tok);
ASTType_T* init_ast_type(ASTTypeKind_T kind, Token_T* tok);

ASTIdentifier_T* init_ast_identifier(Token_T* tok, char* callee);

ASTObj_T* init_ast_obj(ASTObjKind_T kind, Token_T* tok);

void init_ast_prog(ASTProg_T* prog, const char* main_file_path, const char* target_binary);

const char* obj_kind_to_str(ASTObjKind_T kind);
const char* type_kind_to_str(ASTTypeKind_T kind);
char* ast_type_to_str(Context_T* context, char* dest, const ASTType_T* ty, size_t size);
char* ast_id_to_str(char* dest, ASTIdentifier_T* id, size_t size);

char operator_char(ASTNode_T* op);

void merge_ast_progs(ASTProg_T* dest, ASTProg_T* src);

extern const ASTIdentifier_T empty_id;

#endif