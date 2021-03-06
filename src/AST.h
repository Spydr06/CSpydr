#ifndef CSPYDR_AST_H
#define CSPYDR_AST_H

#include "list.h"
#include <stdbool.h>

typedef struct AST_STRUCT AST_T;

typedef struct AST_ROOT_STRUCT
{
    list_T* contents;
} ASTroot_T;

typedef struct AST_COMPOUND_STRUCT
{
    list_T* contents;
} ASTcompound_T;

typedef struct AST_DEF_STRUCT
{
    enum
    {
        VAR, FN
    } type;

    union
    {
        char* dataType;
        char* name;
        bool isFunction;
        list_T* args;
        AST_T* value;
    };
} ASTdef_T;

typedef struct AST_EXPR_STRUCT
{
    enum 
    { 
        CONSTANT, STRING, 
        ADD, SUB, MULT, DIV, ASSIGN, 
        EQUALS, GREATER, LESS, GREATER_EQUALS, LESS_EQUALS, NOT, NOT_EQUALS
    } type;

    union 
    {
        int intValue;
        float floatValue;
        char* strValue;
        struct 
        {
            AST_T* left;
            AST_T* right;
        } op;
    };
} ASTexpr_T;

typedef struct AST_STMT_STRUCT
{
    enum
    {
        IF, RETURN, FOR, WHILE, EXIT
    } type;

    union
    {
        AST_T* value;
        AST_T* condition;
        AST_T* body;
        AST_T* ifBody;
        AST_T* elseBody;
    };
} ASTstmt_T;

struct AST_STRUCT
{
    enum
    {
        ROOT, EXPR, STMT, COMPOUND, DEF
    } type;

    union
    {
        ASTexpr_T* expr;
        ASTstmt_T* stmt;
        ASTcompound_T* compound;
        ASTdef_T* def;
        ASTroot_T* root;
    };
};

AST_T* initAST(int type, int subtype);

#endif