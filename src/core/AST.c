#include "AST.h"
#include <stdarg.h>
#include "../log.h"

#include <string.h>

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
    ast->isFunction = 0;

    return ast;
}

static ASTcompound_T* initCompoundASt(int type)
{
    ASTcompound_T* ast = calloc(1, sizeof(struct AST_COMPOUND_STRUCT));
    ast->contents = initList(sizeof(struct AST_STRUCT));
    return ast;
}

static ASTroot_T* initRootAST(int type)
{
    ASTroot_T* ast = calloc(1, sizeof(struct AST_COMPOUND_STRUCT));
    ast->contents = initList(sizeof(struct AST_STRUCT));
    return ast;
}

static ASTdataType_T* initDataTypeAST(int type)
{
    ASTdataType_T* ast = calloc(1, sizeof(struct AST_DATATYPE_STRUCT));
    ast->type = type;
    return ast;
}

AST_T* initAST(int type, int subtype)
{
    AST_T* ast = calloc(1, sizeof(struct AST_STRUCT));
    ast->type = type;

    ast->root = NULL;
    ast->expr = NULL;
    ast->stmt = NULL;
    ast->compound = NULL;
    ast->def = NULL;

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
        case DATATYPE:
            ast->dataType = initDataTypeAST(subtype);
            break;
        default:
            LOG_ERROR("Undefined AST type %d.\n", type);
            exit(1);
    }
    return ast;
}

char* dataTypeToString(AST_T* ast)
{
    int type = ast->dataType->type;

    switch(type)
    {
        case I8:
            return "I8";
        case I16:
            return "I16";
        case I32:
            return "I32";
        case I64:
            return "I64";
        case U8:
            return "U8";
        case U16:
            return "U16";
        case U32:
            return "U32";
        case U64:
            return "U64";
        case STR:
            return "STR";
        case CHAR:
            return "CHAR";
        case BOOL:
            return "BOOL";
        case VOID:
            return "VOID";
        case VEC: {
            const char* template = "VEC<%s>";
            char* subValue = dataTypeToString(ast->dataType->subtype);
            char* value = calloc(strlen(template) + strlen(subValue) + 1, sizeof(char));
            sprintf(value, template, subValue);
            return value;
        }


        default:
            return "not stringable";
    }
}

char* dataTypeToCTypeToString(AST_T* ast)
{
    int type = ast->dataType->type;

    switch(type)
    {
        case I8:
            return "int8_t";
        case I16:
            return "int16_t";
        case I32:
            return "int32_t";
        case I64:
            return "int64_t";
        case U8:
            return "uint8_t";
        case U16:
            return "uint16_t";
        case U32:
            return "uint32_t";
        case U64:
            return "uint64_t";
        case STR:
            return "char*";
        case CHAR:
            return "char";
        case BOOL:
            return "bool";
        case VOID:
            return "void";
        case VEC: {
            const char* template = "%s*";
            char* subValue = dataTypeToCTypeToString(ast->dataType->subtype);
            char* value = calloc(strlen(template) + strlen(subValue) + 1, sizeof(char));
            sprintf(value, template, subValue);
            return value;
        }

        default:
            return "not stringable";
    }
}
