#include "transpiler.h"
#include "../io/log.h"
#include <string.h>
#include <stdio.h>

#define ADD_STR(str, sec) sec = realloc(sec, (strlen(sec) + strlen(str) + 1) * sizeof(char)); \
                          strcat(sec, str)
#define ADD_INCL(str, tp) ADD_STR(str, tp->inclSection);
#define ADD_TYPE(str, tp) ADD_STR(str, tp->typeSection);
#define ADD_DEF(str, tp)  ADD_STR(str, tp->defSection);
#define ADD_IMPL(str, tp) ADD_STR(str, tp->implSection);

static void generateFile(transpiler_T* tp, ASTFile_T* file);
static void generateGlobal(transpiler_T* tp, ASTGlobal_T* global);
static void generateFunction(transpiler_T* tp, ASTFunction_T* func);
static char* generateType(transpiler_T* tp, ASTType_T* type);
static void generateCompound(transpiler_T* tp, ASTCompound_T* comp);
static void generateStmt(transpiler_T* tp, ASTStmt_T* stmt);
static void generateReturn(transpiler_T* tp, ASTReturn_T* ret);
static char* generateExpr(transpiler_T* tp, ASTExpr_T* expr);

void generateCCode(transpiler_T* tp, ASTProgram_T* ast)
{
    ADD_INCL("#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>", tp);

    for(int i = 0; i < ast->files->size; i++)
    {
        generateFile(tp, (ASTFile_T*) ast->files->items[i]);
    }
}

static void generateFile(transpiler_T* tp, ASTFile_T* file)
{
    for(int i = 0; i < file->globals->size; i++)
    {
        generateGlobal(tp, (ASTGlobal_T*) file->globals->items[i]);
    }

    for(int i = 0; i < file->functions->size; i++)
    {
        generateFunction(tp, (ASTFunction_T*) file->functions->items[i]);
    }
}

static void generateGlobal(transpiler_T* tp, ASTGlobal_T* global)
{
    char* dataType = generateType(tp, global->type);
    ADD_DEF(dataType, tp);
    free(dataType);

    ADD_DEF(" ", tp);
    ADD_DEF(global->name, tp);

    if(global->value)
    {
        ADD_DEF("=", tp);
        char* value = generateExpr(tp, global->value);
        ADD_DEF(value, tp);
        free(value);
    }
    ADD_DEF(";\n", tp);
}

static char* generateFunctionArgs(transpiler_T* tp, list_T* args)
{
    char* argsStr = malloc(sizeof(char));
    argsStr[0] = '\0';

    for(int i = 0; i < args->size; i++)
    {
        ASTArgument_T* arg = (ASTArgument_T*) args->items[i];
        char* dataType = generateType(tp, arg->dataType);
        ADD_STR(dataType, argsStr);
        free(dataType);
        ADD_STR(" ", argsStr);
        ADD_STR(arg->name, argsStr);

        if(i < args->size - 1)
        {
           ADD_STR(",", argsStr);
        }
    }

    return argsStr;
}

static void generateFunction(transpiler_T* tp, ASTFunction_T* func)
{
    char* rt = generateType(tp, func->returnType); // generate the return type;
    char* args = generateFunctionArgs(tp, func->args); //TODO: proper argument handling

    const char* functmp = "%s %s(%s)";
    char* funcDef = calloc(strlen(functmp)
                        + strlen(rt)
                        + strlen(func->name)
                        + strlen(args)
                        + 1, sizeof(char));
    sprintf(funcDef, functmp, rt, func->name, args);

    ADD_DEF(funcDef, tp);
    ADD_DEF(";\n", tp);

    ADD_IMPL(funcDef, tp);
    ADD_IMPL("{\n", tp);

    generateCompound(tp, func->body);

    ADD_IMPL("}\n", tp);

    free(args);
    free(rt);
    free(funcDef);
}

static void generateCompound(transpiler_T* tp, ASTCompound_T* comp)
{
    for(int i = 0; i < comp->stmts->size; i++)
    {
        generateStmt(tp, (ASTStmt_T*) comp->stmts->items[i]);
    }
}

static void generateStmt(transpiler_T* tp, ASTStmt_T* stmt)
{
    switch(stmt->type)
    {
        case STMT_RETURN:
            generateReturn(tp, (ASTReturn_T*) stmt->stmt);
            break;

        default:
            LOG_ERROR_F("Statements of type %d are currently not support transpiling\n", stmt->type);
            exit(1);
    }
}

static void generateReturn(transpiler_T* tp, ASTReturn_T* ret)
{
    ADD_IMPL("return ", tp);
    char* expr = generateExpr(tp, ret->value);
    ADD_IMPL(expr, tp);
    ADD_IMPL(";\n", tp);
    free(expr);
}

static char* generateExpr(transpiler_T* tp, ASTExpr_T* expr)
{
    switch(expr->type)
    {
        case EXPR_NIL:
            return strdup("NULL");
        case EXPR_BOOL_LITERAL:
            return strdup(((ASTBool_T*) expr->expr)->_bool ? "1" : "0");
        case EXPR_INT_LITERAL:
        {
            const char* tmp = "%d";
            char* iStr = calloc(strlen(tmp) + 1, sizeof(char));
            sprintf(iStr, tmp, ((ASTInt_T*) expr->expr)->_int);
            return iStr;
        }
        case EXPR_FLOAT_LITERAL:
        {
            const char* tmp = "%f";
            char* fStr = calloc(strlen(tmp) + 1, sizeof(char));
            sprintf(fStr, tmp, ((ASTFloat_T*) expr->expr)->_float);
            return fStr;
        }
        case EXPR_CHAR_LITERAL:
        {
            const char* tmp = "'%c'";
            char* cStr = calloc(strlen(tmp) + 1, sizeof(char));
            sprintf(cStr, tmp, ((ASTChar_T*) expr->expr)->_char);
            return cStr;
        }
        case EXPR_STRING_LITERAL:
        {
            const char* tmp = "\"%s\"";
            char* sStr = calloc(strlen(tmp) + strlen(((ASTString_T*) expr->expr)->_string) + 1, sizeof(char));
            sprintf(sStr, tmp, ((ASTString_T*) expr->expr)->_string);
            return sStr;
        }

        default:
            LOG_ERROR_F("Expression of type %d currently not support transpiling\n", expr->type);
            exit(1);
    }
}

static char* generateType(transpiler_T* tp, ASTType_T* type)
{
    switch(type->type)
    {
        case AST_I32:
            return strdup("int32_t");
        case AST_I64:
            return strdup("ini64_t");
        case AST_U32:
            return strdup("uint32_t");
        case AST_U64:
            return strdup("uint64_t");
        case AST_F32:
            return strdup("float");
        case AST_F64:
            return strdup("double");
        case AST_CHAR:
            return strdup("char");
        case AST_STRING:
            return strdup("char*");
        case AST_BOOL:
            return strdup("uint8_t");
        case AST_POINTER: {
            char* st = generateType(tp, type->subtype);
            st = realloc(st, (sizeof(st) + 2) * sizeof(char));
            strcat(st, "*");
            return st;
        }
        default:
            LOG_ERROR_F("Transpiling of data type %d is currently not supported", type->type);
            exit(1);
    }
}