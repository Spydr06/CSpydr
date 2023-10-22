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

#include "config.h"

#ifdef __cplusplus
namespace cspydr {
    extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef struct CSPYDR_COMPILER_STRUCT CSpydrCompiler_T;

extern CSpydrCompiler_T* csp_init_compiler();
extern void csp_free_compiler(CSpydrCompiler_T* compiler);

#ifdef __CSPYDR_INTERNAL_USE
// Internal use only!
#define CSPYDR_TYPE(name) name##_T
#else
#define CSPYDR_TYPE(name) CSpydr##name##_T
#endif

typedef enum {
    COMPILER_NONE = 0,
    COMPILER_INIT,
    COMPILER_PARSED,
    COMPILER_OPTIMIZED,
    COMPILER_GENERATED,
    COMPILER_EXECUTED
} CSPYDR_TYPE(CompilerStatus);

extern CSPYDR_TYPE(CompilerStatus) csp_get_status(CSpydrCompiler_T* compiler);
extern const char* csp_status_str(CSPYDR_TYPE(CompilerStatus) status);

typedef struct CSPYDR_CONTEXT_STRUCT CSPYDR_TYPE(Context);

extern CSPYDR_TYPE(Context)* csp_init_context(void);

typedef union CSPYDR_FLAGS_STRUCT {
    struct {
        bool silent : 1;
        bool print_code : 1;
        bool optimize : 1;
        bool embed_debug_info : 1;
        bool from_json : 1;
        bool do_linking : 1;
        bool do_assembling : 1;
        bool do_parsing : 1;
        bool timer_enabled : 1;
        bool clear_cache_after : 1;
        bool read_main_file_on_init : 1;
        bool require_entrypoint : 1;

        uint8_t __unused__ : 4;
    };
    uint16_t flags;
} CSPYDR_TYPE(Flags);

extern CSPYDR_TYPE(Flags) csp_context_get_flags(CSPYDR_TYPE(Context)* context);
extern void csp_context_set_flags(CSPYDR_TYPE(Context)* context, CSPYDR_TYPE(Flags) flags);

#ifdef __CSPYDR_INTERNAL_USE
// Internal use only!
#define CSPYDR_TOKEN(name) \
    TOKEN_##name
#else
#define CSPYDR_TOKEN(name) \
    CSPYDR_TOKEN_##name
#endif

enum CSPYDR_TOKEN_TYPE {
    CSPYDR_TOKEN(ID),           // names, types, etc.
    CSPYDR_TOKEN(MACRO_CALL),   // foo!
    CSPYDR_TOKEN(INFIX_CALL),   // `

    CSPYDR_TOKEN(INT),          // 0, 3, 5, etc.
    CSPYDR_TOKEN(FLOAT),        // 4.2, 3.14, etc.
    CSPYDR_TOKEN(CHAR),         // 'f'
    CSPYDR_TOKEN(STRING),       // "foo"

    CSPYDR_TOKEN(TRUE),         // true
    CSPYDR_TOKEN(FALSE),        // false
    CSPYDR_TOKEN(NIL),          // nil

    CSPYDR_TOKEN(LPAREN),       // (
    CSPYDR_TOKEN(RPAREN),       // )
    CSPYDR_TOKEN(LBRACE),       // {
    CSPYDR_TOKEN(RBRACE),       // }
    CSPYDR_TOKEN(LBRACKET),     // [
    CSPYDR_TOKEN(RBRACKET),     // ]

    CSPYDR_TOKEN(OPERATOR),     // operators like `+`, `-`, `!=`, ...

    CSPYDR_TOKEN(COMMA),        // ,
    CSPYDR_TOKEN(SEMICOLON),    // ;
    CSPYDR_TOKEN(UNDERSCORE),   // _
    CSPYDR_TOKEN(DOLLAR),       // $
    CSPYDR_TOKEN(AT),           // @
    CSPYDR_TOKEN(VA_LIST),      // ...
    CSPYDR_TOKEN(ARROW),        // =>

    CSPYDR_TOKEN(POW_2),        // ²
    CSPYDR_TOKEN(POW_3),        // ³

    CSPYDR_TOKEN(C_ARRAY),      // 'c or 'C

    CSPYDR_TOKEN(ALIGNOF),      // alignof
    CSPYDR_TOKEN(ASM),          // asm
    CSPYDR_TOKEN(BREAK),        // break
    CSPYDR_TOKEN(CONST),        // const
    CSPYDR_TOKEN(CONTINUE),     // continue
    CSPYDR_TOKEN(DEFER),        // defer
    CSPYDR_TOKEN(DO),           // do
    CSPYDR_TOKEN(ELSE),         // else
    CSPYDR_TOKEN(EMBED),        // embed
    CSPYDR_TOKEN(ENUM),         // enum
    CSPYDR_TOKEN(EXTERN),       // extern
    CSPYDR_TOKEN(FN),           // fn
    CSPYDR_TOKEN(FOR),          // for
    CSPYDR_TOKEN(IF),           // if
    CSPYDR_TOKEN(IMPORT),       // import
    CSPYDR_TOKEN(INTERFACE),    // interface
    CSPYDR_TOKEN(LEN),          // len
    CSPYDR_TOKEN(LET),          // let
    CSPYDR_TOKEN(LOOP),         // loop
    CSPYDR_TOKEN(MACRO),        // macro
    CSPYDR_TOKEN(MATCH),        // match
    CSPYDR_TOKEN(NAMESPACE),    // namespace
    CSPYDR_TOKEN(NOOP),         // noop
    CSPYDR_TOKEN(RETURN),       // ret
    CSPYDR_TOKEN(SIZEOF),       // sizeof
    CSPYDR_TOKEN(STRUCT),       // struct
    CSPYDR_TOKEN(TYPE),         // type
    CSPYDR_TOKEN(TYPEOF),       // typeof
    CSPYDR_TOKEN(UNION),        // union
    CSPYDR_TOKEN(UNLESS),       // unless
    CSPYDR_TOKEN(USING),        // using
    CSPYDR_TOKEN(WHILE),        // while
    CSPYDR_TOKEN(WITH),         // with
    CSPYDR_TOKEN(OPERATOR_KW),   // operator keyword

    CSPYDR_TOKEN(CURRENT_FN),   // special token for the __func__! macro

    CSPYDR_TOKEN(ERROR), // error handling token
    CSPYDR_TOKEN(EOF),   // end of file
};

typedef enum CSPYDR_TOKEN_TYPE CSpydrTokenType_T;
typedef struct CSPYDR_TOKEN_STRUCT CSpydrToken_T;

CSpydrToken_T* csp_new_token(CSpydrTokenType_T type, uint32_t line, uint32_t pos, char value[]);
CSpydrTokenType_T csp_token_get_type(CSpydrToken_T* tok);
uint32_t csp_token_get_line(CSpydrToken_T* tok);
uint32_t csp_token_get_position(CSpydrToken_T* tok);
char* csp_token_get_value(CSpydrToken_T* tok);
char* csp_token_get_file(CSpydrToken_T* tok);

#ifdef __CSPYDR_INTERNAL_USE
// Internal use only!
#define CSPYDR_ND(name) \
    ND_##name
#else
#define CSPYDR_ND(name) \
    CSPYDR_ND_##name
#endif

enum CSPYDR_AST_NODE_KIND_ENUM {
    CSPYDR_ND(NOOP),

    // identifiers
    CSPYDR_ND(ID),      // x

    // literals
    CSPYDR_ND(INT),     // 0
    CSPYDR_ND(LONG),
    CSPYDR_ND(ULONG), 
    CSPYDR_ND(FLOAT),   // 0.1
    CSPYDR_ND(DOUBLE),
    CSPYDR_ND(BOOL),    // true, false
    CSPYDR_ND(CHAR),    // 'x'
    CSPYDR_ND(STR),     // "..."
    CSPYDR_ND(NIL),     // nil

    CSPYDR_ND(ARRAY),   // [2, 4, ...]
    CSPYDR_ND(STRUCT),  // {3, 4, ...}

    // operators
    CSPYDR_ND(ADD),     // +
    CSPYDR_ND(SUB),     // -
    CSPYDR_ND(MUL),     // *
    CSPYDR_ND(DIV),     // /
    CSPYDR_ND(MOD),     // %

    CSPYDR_ND(NEG),     // unary -
    CSPYDR_ND(BIT_NEG), // unary ~
    CSPYDR_ND(NOT),     // unary !
    CSPYDR_ND(REF),     // unary &
    CSPYDR_ND(DEREF),   // unary *

    CSPYDR_ND(EQ),      // ==
    CSPYDR_ND(NE),      // !=
    CSPYDR_ND(GT),      // >
    CSPYDR_ND(GE),      // >=
    CSPYDR_ND(LT),      // <
    CSPYDR_ND(LE),      // <=

    CSPYDR_ND(AND), // &&
    CSPYDR_ND(OR),  // ||

    CSPYDR_ND(LSHIFT),  // <<
    CSPYDR_ND(RSHIFT),  // >>
    CSPYDR_ND(XOR),     // ^
    CSPYDR_ND(BIT_OR),  // |
    CSPYDR_ND(BIT_AND), // &

    CSPYDR_ND(INC),     // ++
    CSPYDR_ND(DEC),     // --

    CSPYDR_ND(CLOSURE), // ()
    CSPYDR_ND(ASSIGN),  // x = y

    CSPYDR_ND(MEMBER),  // x.y
    CSPYDR_ND(CALL),    // x(y, z)
    CSPYDR_ND(INDEX),   // x[y]
    CSPYDR_ND(CAST),    // x:i32

    CSPYDR_ND(SIZEOF),  // sizeof x
    CSPYDR_ND(ALIGNOF), // alignof x

    CSPYDR_ND(PIPE),    // x |> y
    CSPYDR_ND(HOLE),    // $
    CSPYDR_ND(LAMBDA),  // |x: i32| => {}

    CSPYDR_ND(ELSE_EXPR), // x else y

    CSPYDR_ND(TYPE_EXPR), // type expressions like: "(type) T == U" or "(type) reg_class(T)"

    // statements
    CSPYDR_ND(BLOCK),         // {...}
    CSPYDR_ND(IF),            // if x {}
    CSPYDR_ND(TERNARY),       // if x => y <> z
    CSPYDR_ND(LOOP),          // loop {}
    CSPYDR_ND(WHILE),         // while x {}
    CSPYDR_ND(FOR),           // for let i: i32 = 0; i < x; i++ {}
    CSPYDR_ND(FOR_RANGE),     // for x..y {}
    CSPYDR_ND(MATCH),         // match x {}
    CSPYDR_ND(MATCH_TYPE),    // match (type) T {}
    CSPYDR_ND(CASE),          // x => {} !!only in match statements!!
    CSPYDR_ND(CASE_TYPE),     // i32 => {}
    CSPYDR_ND(RETURN),        // ret x;
    CSPYDR_ND(EXPR_STMT),     // "executable" expressions
    CSPYDR_ND(BREAK),         // break;
    CSPYDR_ND(CONTINUE),      // continue;
    CSPYDR_ND(DO_UNLESS),     // do {} unless x;
    CSPYDR_ND(DO_WHILE),      // do {} while x;
    CSPYDR_ND(LEN),           // len x
    CSPYDR_ND(USING),         // using x::y
    CSPYDR_ND(WITH),          // with x = y {}
    CSPYDR_ND(STRUCT_MEMBER), // struct members
    CSPYDR_ND(EMBED_STRUCT),  // embedded Struct
    CSPYDR_ND(DEFER),         // defer {}
    CSPYDR_ND(EXTERN_C_BLOCK), // extern "C" {}

    CSPYDR_ND(ASM), // inline assembly

    // builtin functions used exclusively in type expressions
    CSPYDR_ND(BUILTIN_REG_CLASS),
    CSPYDR_ND(BUILTIN_IS_INT),
    CSPYDR_ND(BUILTIN_IS_UINT),
    CSPYDR_ND(BUILTIN_IS_FLOAT),
    CSPYDR_ND(BUILTIN_IS_POINTER),
    CSPYDR_ND(BUILTIN_IS_ARRAY),
    CSPYDR_ND(BUILTIN_IS_STRUCT),
    CSPYDR_ND(BUILTIN_IS_UNION),
    CSPYDR_ND(BUILTIN_TO_STR),

    CSPYDR_ND(KIND_LEN)
};

#ifdef __CSPYDR_INTERNAL_USE
// Internal use only!
#define CSPYDR_TY(name) \
    TY_##name
#else
#define CSPYDR_TY(name) \
    CSPYDR_TY_##name
#endif

enum CSPYDR_AST_TYPE_KIND_ENUM {
    CSPYDR_TY(I8),      // i8
    CSPYDR_TY(I16),     // i16
    CSPYDR_TY(I32),     // i32
    CSPYDR_TY(I64),     // i64

    CSPYDR_TY(U8),      // u8
    CSPYDR_TY(U16),     // u16
    CSPYDR_TY(U32),     // u32
    CSPYDR_TY(U64),     // u64

    CSPYDR_TY(F32),     // f32
    CSPYDR_TY(F64),     // f64
    CSPYDR_TY(F80),     // f80

    CSPYDR_TY(BOOL),    // bool
    CSPYDR_TY(VOID),    // void
    CSPYDR_TY(CHAR),    // char

    CSPYDR_TY(PTR),     // &x
    CSPYDR_TY(ARRAY),   // x[y]
    CSPYDR_TY(VLA),     // x[]
    CSPYDR_TY(C_ARRAY), // x'c[y]
    CSPYDR_TY(STRUCT),  // struct {}
    CSPYDR_TY(ENUM),    // enum {}

    CSPYDR_TY(FN),      // fn(x): y

    CSPYDR_TY(UNDEF),   // <identifier>
    CSPYDR_TY(TYPEOF),  // typeof x
    CSPYDR_TY(TEMPLATE), // template types temporarily used during parsing
    CSPYDR_TY(INTERFACE),   // interface
    
    CSPYDR_TY(KIND_LEN)
};

#ifdef __CSPYDR_INTERNAL_USE
// Internal use only!
#define CSPYDR_OBJ(name) \
    OBJ_##name
#else
#define CSPYDR_OBJ(name) \
    CSPYDR_OBJ_##name
#endif

enum CSPYDR_AST_OBJ_KIND_ENUM {
    CSPYDR_OBJ(GLOBAL)      = 0b00000001, // global variable
    CSPYDR_OBJ(LOCAL)       = 0b00000010, // local variable
    CSPYDR_OBJ(FUNCTION)    = 0b00000100, // function
    CSPYDR_OBJ(FN_ARG)      = 0b00001000, // function argument
    CSPYDR_OBJ(TYPEDEF)     = 0b00010000, // datatype definition
    CSPYDR_OBJ(NAMESPACE)   = 0b00100000, // namespace
    CSPYDR_OBJ(ENUM_MEMBER) = 0b01000000, // member of an `enum` data type

    //! internal:
    CSPYDR_OBJ(LAMBDA)      = 0b10000000,      // lambda implementation used internally

    CSPYDR_OBJ(ANY)         = 0b11111111,
    CSPYDR_OBJ(KIND_LEN)
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