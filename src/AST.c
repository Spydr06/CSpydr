#include "AST.h"
#include <stdarg.h>
#include "log.h"

static ASTexpr_T* initExprAST(int type)
{
    ASTexpr_T* ast = calloc(1, sizeof(struct AST_EXPR_STRUCT));
    ast->type = type;

    return ast;
}

static ASTstmt_T* initStmtAST(int type)
{
    ASTstmt_T* ast = calloc(1, sizeof(struct AST_STMT_STRUCT));
    ast->type = type;

    return ast;
}

static ASTdef_T* initDefAST(int type)
{
    ASTdef_T* ast = calloc(1, sizeof(struct AST_DEF_STRUCT));
    ast->type = type;

    return ast;
}

static ASTcompound_T* initCompoundASt(int type)
{
    ASTcompound_T* ast = calloc(1, sizeof(struct AST_COMPOUND_STRUCT));
    ast->contents = initList(sizeof(struct AST_STRUCT*));
    return ast;
}

static ASTroot_T* initRootAST(int type)
{
    ASTroot_T* ast = calloc(1, sizeof(struct AST_COMPOUND_STRUCT));
    ast->contents = initList(sizeof(struct AST_STRUCT*));
    return ast;
}

AST_T* initAST(int type, int subtype)
{
    AST_T* ast = calloc(1, sizeof(struct AST_STRUCT));
    ast->type = type;

    switch(ast->type)
    {
        case ROOT:
            ast->root = initRootAST(subtype);
            break;
        case EXPR:
            ast->expr = initExprAST(subtype);
            break;
        case STMT:
            ast->stmt = initStmtAST(subtype);
            break;
        case COMPOUND:
            ast->compound = initCompoundASt(subtype);
            break;
        case DEF:
            ast->def = initDefAST(subtype);
            break;
        default:
            LOG_ERROR("Undefined AST type %d.\n", type);
            exit(1);
    }
    return ast;
}