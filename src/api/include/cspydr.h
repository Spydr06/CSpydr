/*
    LIBCSPC API HEADERS - Interface for communicating with the compiler of the CSpydr programming language
    
    Copyright (c) 2021 - 2022 Spydr06
    This code and all code of CSpydr is licensed under the MIT license.
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    The software is provided "as is", without warranty of any kind.

    cspydr.h features an API for CSpydr and its compiler cspc for C/C++
*/

#ifndef __CSPYDR_H
#define __CSPYDR_H

#ifdef __cplusplus
namespace cspydr {
    extern "C" {
#endif

#include <stdint.h>

typedef struct CSPYDR_COMPILER_STRUCT CSpydrCompiler_T;

extern CSpydrCompiler_T* csp_init_compiler();
extern void csp_free_compiler(CSpydrCompiler_T* compiler);

typedef enum {
    COMPILER_NONE = 0,
    COMPILER_INIT,
    COMPILER_PARSED,
    COMPILER_OPTIMIZED,
    COMPILER_GENERATED,
    COMPILER_EXECUTED
} CompilerStatus_T;

extern CompilerStatus_T csp_get_status(CSpydrCompiler_T* compiler);
const char* csp_status_str(CompilerStatus_T status);

enum CSPYDR_TOKEN_TYPE {
    CSPYDR_TOKEN_ID,           // names, types, etc.
    CSPYDR_TOKEN_MACRO_CALL,   // foo!
    CSPYDR_TOKEN_INFIX_CALL,   // `

    CSPYDR_TOKEN_INT,          // 0, 3, 5, etc.
    CSPYDR_TOKEN_FLOAT,        // 4.2, 3.14, etc.
    CSPYDR_TOKEN_CHAR,         // 'f'
    CSPYDR_TOKEN_STRING,       // "foo"

    CSPYDR_TOKEN_TRUE,         // true
    CSPYDR_TOKEN_FALSE,        // false
    CSPYDR_TOKEN_NIL,          // nil

    CSPYDR_TOKEN_LPAREN,       // (
    CSPYDR_TOKEN_RPAREN,       // )
    CSPYDR_TOKEN_LBRACE,       // {
    CSPYDR_TOKEN_RBRACE,       // }
    CSPYDR_TOKEN_LBRACKET,     // [
    CSPYDR_TOKEN_RBRACKET,     // ]

    CSPYDR_TOKEN_GT,           // >
    CSPYDR_TOKEN_LT,           // <
    CSPYDR_TOKEN_EQ,           // ==
    CSPYDR_TOKEN_NOT_EQ,       // !=
    CSPYDR_TOKEN_GT_EQ,        // >=
    CSPYDR_TOKEN_LT_EQ,        // <=
    CSPYDR_TOKEN_BANG,         // !

    CSPYDR_TOKEN_PLUS,         // +
    CSPYDR_TOKEN_MINUS,        // -
    CSPYDR_TOKEN_STAR,         // *
    CSPYDR_TOKEN_SLASH,        // /
    CSPYDR_TOKEN_PERCENT,      // %
    CSPYDR_TOKEN_ARROW,        // =>
    CSPYDR_TOKEN_AND,          // &&
    CSPYDR_TOKEN_OR,           // ||
    CSPYDR_TOKEN_BIT_OR,       // |
    CSPYDR_TOKEN_PIPE,         // |>
    CSPYDR_TOKEN_REF,          // &
    CSPYDR_TOKEN_TILDE,        // ~
    CSPYDR_TOKEN_VA_LIST,      // ...

    CSPYDR_TOKEN_INC,          // ++
    CSPYDR_TOKEN_DEC,          // --

    CSPYDR_TOKEN_ASSIGN,       // =
    CSPYDR_TOKEN_ADD,          // +=
    CSPYDR_TOKEN_SUB,          // -=
    CSPYDR_TOKEN_MULT,         // *=
    CSPYDR_TOKEN_DIV,          // /=
    CSPYDR_TOKEN_MOD,          // %=

    CSPYDR_TOKEN_XOR,          // ^
    CSPYDR_TOKEN_XOR_ASSIGN,   // ^=
    CSPYDR_TOKEN_LSHIFT,       // <<
    CSPYDR_TOKEN_RSHIFT,       // >>
    
    CSPYDR_TOKEN_LSHIFT_ASSIGN,// <<=
    CSPYDR_TOKEN_RSHIFT_ASSIGN,// >>=

    CSPYDR_TOKEN_BIT_AND_ASSIGN,// &=
    CSPYDR_TOKEN_BIT_OR_ASSIGN, // |=

    CSPYDR_TOKEN_COLON,        // :
    CSPYDR_TOKEN_COMMA,        // ,
    CSPYDR_TOKEN_DOT,          // .
    CSPYDR_TOKEN_SEMICOLON,    // ;
    CSPYDR_TOKEN_UNDERSCORE,   // _
    CSPYDR_TOKEN_DOLLAR,       // $

    CSPYDR_TOKEN_STATIC_MEMBER,// ::

    CSPYDR_TOKEN_POW_2,        // ²
    CSPYDR_TOKEN_POW_3,        // ³

    CSPYDR_TOKEN_C_ARRAY,      // 'c or 'C

    CSPYDR_TOKEN_IF,           // if
    CSPYDR_TOKEN_ELSE,         // else
    CSPYDR_TOKEN_LOOP,         // loop
    CSPYDR_TOKEN_RETURN,       // ret
    CSPYDR_TOKEN_MATCH,        // match
    CSPYDR_TOKEN_FN,           // fn
    CSPYDR_TOKEN_LET,          // let
    CSPYDR_TOKEN_TYPE,         // type
    CSPYDR_TOKEN_STRUCT,       // struct
    CSPYDR_TOKEN_UNION,        // union
    CSPYDR_TOKEN_ENUM,         // enum
    CSPYDR_TOKEN_IMPORT,       // import
    CSPYDR_TOKEN_EXTERN,       // extern
    CSPYDR_TOKEN_MACRO,        // macro
    CSPYDR_TOKEN_CONST,        // const
    CSPYDR_TOKEN_NAMESPACE,    // namespace
    CSPYDR_TOKEN_SIZEOF,       // sizeof
    CSPYDR_TOKEN_TYPEOF,       // typeof
    CSPYDR_TOKEN_ALIGNOF,      // alignof
    CSPYDR_TOKEN_WHILE,        // while
    CSPYDR_TOKEN_FOR,          // for
    CSPYDR_TOKEN_CONTINUE,     // continue
    CSPYDR_TOKEN_BREAK,        // break
    CSPYDR_TOKEN_NOOP,         // noop
    CSPYDR_TOKEN_LEN,          // len
    CSPYDR_TOKEN_ASM,          // asm
    CSPYDR_TOKEN_USING,        // using
    CSPYDR_TOKEN_WITH,         // with
    CSPYDR_TOKEN_DO,           // do
    CSPYDR_TOKEN_UNLESS,       // unless

    CSPYDR_TOKEN_CURRENT_FN,   // special token for the __func__! macro

    // builtin functions used exclusively in type expressions
    CSPYDR_TOKEN_BUILTIN_REG_CLASS,
    CSPYDR_TOKEN_BUILTIN_IS_INT,
    CSPYDR_TOKEN_BUILTIN_IS_UINT,
    CSPYDR_TOKEN_BUILTIN_IS_FLOAT,
    CSPYDR_TOKEN_BUILTIN_IS_POINTER,
    CSPYDR_TOKEN_BUILTIN_IS_ARRAY,
    CSPYDR_TOKEN_BUILTIN_IS_STRUCT,
    CSPYDR_TOKEN_BUILTIN_IS_UNION,
    CSPYDR_TOKEN_BUILTIN_TO_STR,

    CSPYDR_TOKEN_ERROR, // error handling token
    CSPYDR_TOKEN_EOF,   // end of file
};

typedef enum CSPYDR_TOKEN_TYPE CSpydrTokenType_T;
typedef struct CSPYDR_TOKEN_STRUCT CSpydrToken_T;

CSpydrToken_T* csp_new_token(CSpydrTokenType_T type, uint32_t line, uint32_t pos, char value[]);
CSpydrTokenType_T csp_token_get_type(CSpydrToken_T* tok);
uint32_t csp_token_get_line(CSpydrToken_T* tok);
uint32_t csp_token_get_position(CSpydrToken_T* tok);
char* csp_token_get_value(CSpydrToken_T* tok);
char* csp_token_get_file(CSpydrToken_T* tok);

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

typedef struct AST_NODE_STRUCT       CSpydrASTNode_T;
typedef struct AST_OBJ_STRUCT        CSpydrASTObj_T;
typedef struct AST_TYPE_STRUCT       CSpydrASTType_T;
typedef struct AST_IDENTIFIER_STRUCT CSpydrASTIdentifier_T;
typedef struct AST_PROG_STRUCT       CSpydrASTProg_T;

#ifdef __cplusplus
    } // extern "C"
} // namespace cspydr
#endif

#endif // __CSPYDR_H