#ifndef CSPYDR_IR_H
#define CSPYDR_IR_H

#include "config.h"
#include "lexer/token.h"
#include "list.h"
#include "util.h"

#define IR_LOCAL_ID(id) ((IRIdentifier_T){.kind = IR_ID_LOCAL, .local_id = (u32)(id)})

typedef struct IR_REGISTER_STRUCT IRRegister_T;
typedef struct IR_LITERAL_STRUCT IRLiteral_T;
typedef struct IR_LVALUE_STRUCT IRLValue_T;
typedef struct IR_STMT_STRUCT IRStmt_T;
typedef struct IR_TYPE_STRUCT IRType_T;

typedef struct IR_FUNCTION_STRUCT IRFunction_T;
typedef struct IR_GLOBAL_STRUCT IRGlobal_T;
typedef struct IR_PARAMETER_STRUCT IRParameter_T;

struct IR_REGISTER_STRUCT {
    uint32_t id;
};

typedef enum IR_LVALUE_KIND_ENUM : u8 {
    IR_LVALUE_ALLOCA,
    IR_LVALUE_POP,
    IR_LVALUE_PARAMETER,
    IR_LVALUE_GLOBAL,
    IR_LVALUE_GLOBAL_PTR,
    IR_LVALUE_FUNC_PTR
} IRLValueKind_T;

struct IR_LVALUE_STRUCT {
    IRLValueKind_T kind;

    IRType_T* type;

    union {
        IRLiteral_T* push_from;
        u32 parameter_num;
        const char* global_id;
        const char* function_id;
    };
};

typedef enum IR_LITERAL_KIND_ENUM : u8 {
    IR_LITERAL_VOID, // nothing

    IR_LITERAL_I8,
    IR_LITERAL_U8,
    IR_LITERAL_I32,
    IR_LITERAL_I64,
    IR_LITERAL_U64,
    
    IR_LITERAL_F32,
    IR_LITERAL_F64,

    IR_LITERAL_REG
} IRLiteralKind_T;

void init_ir_literal(IRLiteral_T* dst, IRLiteralKind_T kind, IRType_T* type);

struct IR_LITERAL_STRUCT {
    IRLiteralKind_T kind;

    IRType_T* type;
    
    union {
        i8 i8_lit;
        u8 u8_lit;
        i32 i32_lit;
        i64 i64_lit;
        u64 u64_lit;

        f32 f32_lit;
        f64 f64_lit;

        IRRegister_T reg;
    };
};

typedef enum IR_STMT_KIND_ENUM : u8 {
    IR_STMT_RETURN,
    IR_STMT_LABEL,
    IR_STMT_GOTO,
    IR_STMT_GOTO_IF,
    IR_STMT_DECL,
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
            bool negated;
            IRLiteral_T condition;
        } goto_if;
        struct {
            IRLiteral_T lit;
        } _return;
        struct {
            IRRegister_T reg;
            IRLValue_T value;
        } decl;
    };
};

IRStmt_T* init_ir_stmt(Context_T* context, IRStmtKind_T kind);

struct IR_PARAMETER_STRUCT {
    Token_T* tok;
    IRType_T* type;
};

IRParameter_T* init_ir_parameter(Context_T* context, Token_T* tok, IRType_T* type);

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

    IRType_T* type;
    IRLiteral_T value;

    union {
        struct {
            bool is_extern : 1;
            bool is_const : 1;
        };
        u8 __flags__;
    };
};

IRGlobal_T* init_ir_global(Context_T* context, Token_T* tok, const char* mangled_id, bool is_extern, bool is_const, IRType_T* type, IRLiteral_T value);

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

