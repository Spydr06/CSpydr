#ifndef __CSPYDR_AST_ENUMS_H
#define __CSPYDR_AST_ENUMS_H

enum CSPYDR_AST_NODE_KIND_ENUM {
    CSPYDR_ND_NOOP,

    // identifiers
    CSPYDR_ND_ID,      // x

    // literals
    CSPYDR_ND_INT,     // 0
    CSPYDR_ND_LONG,
    CSPYDR_ND_ULONG, 
    CSPYDR_ND_FLOAT,   // 0.1
    CSPYDR_ND_DOUBLE,
    CSPYDR_ND_BOOL,    // true, false
    CSPYDR_ND_CHAR,    // 'x'
    CSPYDR_ND_STR,     // "..."
    CSPYDR_ND_NIL,     // nil

    CSPYDR_ND_ARRAY,   // [2, 4, ...]
    CSPYDR_ND_STRUCT,  // {3, 4, ...}

    // operators
    CSPYDR_ND_ADD,     // +
    CSPYDR_ND_SUB,     // -
    CSPYDR_ND_MUL,     // *
    CSPYDR_ND_DIV,     // /
    CSPYDR_ND_MOD,     // %

    CSPYDR_ND_NEG,     // unary -
    CSPYDR_ND_BIT_NEG, // unary ~
    CSPYDR_ND_NOT,     // unary !
    CSPYDR_ND_REF,     // unary &
    CSPYDR_ND_DEREF,   // unary *

    CSPYDR_ND_EQ,      // ==
    CSPYDR_ND_NE,      // !=
    CSPYDR_ND_GT,      // >
    CSPYDR_ND_GE,      // >=
    CSPYDR_ND_LT,      // <
    CSPYDR_ND_LE,      // <=

    CSPYDR_ND_AND, // &&
    CSPYDR_ND_OR,  // ||

    CSPYDR_ND_LSHIFT,  // <<
    CSPYDR_ND_RSHIFT,  // >>
    CSPYDR_ND_XOR,     // ^
    CSPYDR_ND_BIT_OR,  // |
    CSPYDR_ND_BIT_AND, // &

    CSPYDR_ND_INC,     // ++
    CSPYDR_ND_DEC,     // --

    CSPYDR_ND_CLOSURE, // ()
    CSPYDR_ND_ASSIGN,  // x = y

    CSPYDR_ND_MEMBER,  // x.y
    CSPYDR_ND_CALL,    // x(y, z)
    CSPYDR_ND_INDEX,   // x[y]
    CSPYDR_ND_CAST,    // x:i32

    CSPYDR_ND_SIZEOF,  // sizeof x
    CSPYDR_ND_ALIGNOF, // alignof x

    CSPYDR_ND_PIPE,    // x |> y
    CSPYDR_ND_HOLE,    // $
    CSPYDR_ND_LAMBDA,  // |x: i32| => {}

    CSPYDR_ND_ELSE_EXPR, // x else y

    CSPYDR_ND_TYPE_EXPR, // type expressions like: "(type) T == U" or "(type) reg_class(T)"

    // statements
    CSPYDR_ND_BLOCK,         // {...}https://github.com/deter0/ActivateWindows2
    CSPYDR_ND_IF,            // if x {}
    CSPYDR_ND_TERNARY,       // if x => y <> z
    CSPYDR_ND_LOOP,          // loop {}
    CSPYDR_ND_WHILE,         // while x {}
    CSPYDR_ND_FOR,           // for let i: i32 = 0; i < x; i++ {}
    CSPYDR_ND_MATCH,         // match x {}
    CSPYDR_ND_MATCH_TYPE,    // match (type) T {}
    CSPYDR_ND_CASE,          // x => {} !!only in match statements!!
    CSPYDR_ND_CASE_TYPE,     // i32 => {}
    CSPYDR_ND_RETURN,        // ret x;
    CSPYDR_ND_EXPR_STMT,     // "executable" expressions
    CSPYDR_ND_BREAK,         // break;
    CSPYDR_ND_CONTINUE,      // continue;
    CSPYDR_ND_DO_UNLESS,     // do {} unless x;
    CSPYDR_ND_DO_WHILE,      // do {} while x;
    CSPYDR_ND_LEN,           // len x
    CSPYDR_ND_USING,         // using x::y
    CSPYDR_ND_WITH,          // with x = y {}
    CSPYDR_ND_STRUCT_MEMBER, // struct members

    CSPYDR_ND_ASM, // inline assembly

    CSPYDR_ND_KIND_LEN
};

enum CSPYDR_AST_TYPE_KIND_ENUM {
    CSPYDR_TY_I8,      // i8
    CSPYDR_TY_I16,     // i16
    CSPYDR_TY_I32,     // i32
    CSPYDR_TY_I64,     // i64

    CSPYDR_TY_U8,      // u8
    CSPYDR_TY_U16,     // u16
    CSPYDR_TY_U32,     // u32
    CSPYDR_TY_U64,     // u64

    CSPYDR_TY_F32,     // f32
    CSPYDR_TY_F64,     // f64
    CSPYDR_TY_F80,     // f80

    CSPYDR_TY_BOOL,    // bool
    CSPYDR_TY_VOID,    // void
    CSPYDR_TY_CHAR,    // char

    CSPYDR_TY_PTR,     // &x
    CSPYDR_TY_ARRAY,   // x[y]
    CSPYDR_TY_VLA,     // x[]
    CSPYDR_TY_C_ARRAY, // x'c[y]
    CSPYDR_TY_STRUCT,  // struct {}
    CSPYDR_TY_ENUM,    // enum {}

    CSPYDR_TY_FN,      // fn(x): y

    CSPYDR_TY_UNDEF,   // <identifier>
    CSPYDR_TY_TYPEOF,  // typeof x
    CSPYDR_TY_TEMPLATE, // template types temporarily used during parsing
    
    CSPYDR_TY_KIND_LEN
};

enum CSPYDR_AST_OBJ_KIND_ENUM {
    CSPYDR_OBJ_GLOBAL,      // global variable
    CSPYDR_OBJ_LOCAL,       // local variable
    CSPYDR_OBJ_FUNCTION,    // function
    CSPYDR_OBJ_FN_ARG,      // function argument
    CSPYDR_OBJ_TYPEDEF,     // datatype definition
    CSPYDR_OBJ_NAMESPACE,   // namespace
    CSPYDR_OBJ_ENUM_MEMBER, // member of an `enum` data type

    //! internal:
    CSPYDR_OBJ_LAMBDA,      // lambda implementation used internally

    CSPYDR_OBJ_KIND_LEN
};

#endif // __CSPYDR_AST_ENUMS_H