#ifndef CSPYDR_IR_H
#define CSPYDR_IR_H

#include "config.h"
#include "lexer/token.h"
#include "list.h"
#include "util.h"

#define IR_LOCAL_ID(id) ((IRIdentifier_T){.kind = IR_ID_LOCAL, .local_id = (u32)(id)})

typedef struct IR_EXPR_STRUCT IRExpr_T;
typedef struct IR_STMT_STRUCT IRStmt_T;
typedef struct IR_TYPE_STRUCT IRType_T;

typedef struct IR_FUNCTION_STRUCT IRFunction_T;
typedef struct IR_GLOBAL_STRUCT IRGlobal_T;
typedef struct IR_LOCAL_STRUCT IRLocal_T;
typedef struct IR_PARAMETER_STRUCT IRParameter_T;

typedef enum IR_EXPR_KIND_ENUM : u8 {
    IR_EXPR_I8_LIT,
    IR_EXPR_U8_LIT,
    IR_EXPR_I32_LIT,
    IR_EXPR_I64_LIT,
    IR_EXPR_U64_LIT,
    
    IR_EXPR_F32_LIT,
    IR_EXPR_F64_LIT,

    IR_EXPR_ARRAY_LIT,

    IR_EXPR_CAST,
    IR_EXPR_ID,

    IR_EXPR_CMP,
} IRExprKind_T;

typedef struct IR_IDENTIFIER_STRUCT {
    enum : u8 {
        IR_ID_GLOBAL,
        IR_ID_LOCAL,
        IR_ID_PARAMETER,
        IR_ID_FUNCTION,
    } kind;
    union {
        u32 local_id;
        u32 parameter_id;
        const char* mangled_id;
    };
} IRIdentifier_T;

struct IR_EXPR_STRUCT {
    IRExprKind_T kind;

    IRType_T* idx;

    IRType_T* type;
    
    union {
        i8 i8_lit;
        u8 u8_lit;
        i32 i32_lit;
        i64 i64_lit;
        u64 u64_lit;

        f32 f32_lit;
        f64 f64_lit;

        List_T* args;
        struct {
            IRExpr_T* expr;
        } unary;
        struct {
            IRExpr_T* left;
            IRExpr_T* right;
        } binary;
        
        IRIdentifier_T ident;
    };
};

IRExpr_T* init_ir_expr(Context_T* context, IRExprKind_T kind, IRType_T* type);

typedef enum IR_STMT_KIND_ENUM : u8 {
    IR_STMT_RETURN,
    IR_STMT_LABEL,
    IR_STMT_GOTO,
    IR_STMT_GOTO_IF,
    IR_STMT_ASM,
    IR_STMT_PUSH_LOCAL,
    IR_STMT_POP_LOCAL,
    IR_STMT_ASSIGN,
} IRStmtKind_T;

struct IR_STMT_STRUCT {
    IRStmtKind_T kind;

    union {
        struct {
            u32 id;
        } label;
        struct {
            u32 label_id;
        } _goto;
        struct {
            u32 label_id;
            IRExpr_T* condition;
            bool negated;
        } goto_if;
        struct {
            IRExpr_T* value;
        } _return;
        struct {
            List_T* args; 
        } _asm;
        struct {
            IRLocal_T* local;
        } push_l;
        struct {
        } pop_l;
        struct {
            IRExpr_T* dest;
            IRExpr_T* value;
        } assign;
    };
};

IRStmt_T* init_ir_stmt(Context_T* context, IRStmtKind_T kind);

struct IR_LOCAL_STRUCT {
    u32 id;
    bool temporary;
    IRType_T* type;
};

IRLocal_T* init_ir_local(Context_T* context, u32 id, IRType_T* type);

struct IR_PARAMETER_STRUCT {
    u32 id;
    IRType_T* type;
};

IRParameter_T* init_ir_parameter(Context_T* context, u32 id, IRType_T* type);

struct IR_FUNCTION_STRUCT {
    Token_T* tok;
    const char* mangled_id;

    List_T* params;

    union {
        struct {
            bool variadic : 1;
            bool is_extern : 1;
        };
        u8 __flags__;
    };

    IRType_T* return_type;

    List_T* stmts;
    List_T* locals;
};

IRFunction_T* init_ir_function(Context_T* context, Token_T* tok, const char* mangled_id, bool is_extern, bool variadic, IRType_T* return_type);

struct IR_GLOBAL_STRUCT {
    Token_T* tok;
    const char* mangled_id;

    union {
        struct {
            bool is_extern : 1;
            bool is_const : 1;
        };
        u8 __flags__;
    };

    IRType_T* type;
    IRExpr_T* value;
};

IRGlobal_T* init_ir_global(Context_T* context, Token_T* tok, const char* mangled_id, bool is_extern, bool is_const, IRType_T* type, IRExpr_T* value);

typedef enum IR_TYPE_KIND_ENUM : u8 {
    IR_TYPE_I8,
    IR_TYPE_I16,
    IR_TYPE_I32,
    IR_TYPE_I64,

    IR_TYPE_U8,
    IR_TYPE_U16,
    IR_TYPE_U32,
    IR_TYPE_U64,

    IR_TYPE_F32,
    IR_TYPE_F64,
    IR_TYPE_F80,

    IR_TYPE_VOID,

    IR_TYPE_C_ARRAY,
    IR_TYPE_ARRAY,
    IR_TYPE_VLA,
    
    IR_TYPE_PTR,
    IR_TYPE_FN,

    IR_TYPE_STRUCT,
    IR_TYPE_UNION
} IRTypeKind_T;

typedef struct IR_TYPE_FIELD_STRUCT {
    const char* id;
    IRType_T* type;
    u32 offset;
} IRTypeField_T;

struct IR_TYPE_STRUCT {
    IRTypeKind_T kind;

    u32 size;
    u32 align;

    IRType_T* base;

    union {
        List_T* fields;
        u64 indices;
        struct {
            List_T* arguments;
            bool variadic;
        } fn;
    };
};

typedef struct IR_STRUCT {
    List_T* files;

    List_T* types;
    List_T* globals;
    List_T* functions;
} IR_T;

void ir_init(IR_T* ir, Context_T* context);

#endif

