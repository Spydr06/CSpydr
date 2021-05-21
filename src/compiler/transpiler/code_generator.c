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

static void generateTypedef(transpiler_T* tp, ASTTypedef_T* tdef);
static void generateGlobal(transpiler_T* tp, ASTGlobal_T* global);
static void generateFunction(transpiler_T* tp, ASTFunction_T* func);
static void generateCompound(transpiler_T* tp, ASTCompound_T* comp);

static void generateStmt(transpiler_T* tp, ASTStmt_T* stmt);
static void generateReturn(transpiler_T* tp, ASTReturn_T* ret);
static void generateLocal(transpiler_T* tp, ASTLocal_T* loc);
static void generateIf(transpiler_T* tp, ASTIf_T* ifs);
static void generateLoop(transpiler_T* tp, ASTLoop_T* loop);
static void generateMatch(transpiler_T* tp, ASTMatch_T* match);

static char* generateType(transpiler_T* tp, ASTType_T* type);
static char* generateStructType(transpiler_T* tp, ASTStructType_T* st);
static char* generateEnumType(transpiler_T* tp, ASTEnumType_T* et);

static char* generateExpr(transpiler_T* tp, ASTExpr_T* expr);
static char* generateId(transpiler_T* tp, ASTIdentifier_T* id);
static char* generateInfixExpression(transpiler_T* tp, ASTInfix_T* ifx);
static char* generatePrefixExpression(transpiler_T* tp, ASTPrefix_T* pfx);
static char* generatePostfixExpression(transpiler_T* tp, ASTPostfix_T* pfx);
static char* generateIndexExpression(transpiler_T* tp, ASTIndex_T* idx);

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
    for(int i = 0; i < file->types->size; i++)
    {
        generateTypedef(tp, (ASTTypedef_T*) file->types->items[i]);
    }

    for(int i = 0; i < file->globals->size; i++)
    {
        generateGlobal(tp, (ASTGlobal_T*) file->globals->items[i]);
    }

    for(int i = 0; i < file->functions->size; i++)
    {
        generateFunction(tp, (ASTFunction_T*) file->functions->items[i]);
    }
}

static void generateTypedef(transpiler_T* tp, ASTTypedef_T* tdef)
{
    ADD_TYPE("typedef ", tp);
    char* dataType = generateType(tp, tdef->dataType);
    ADD_TYPE(dataType, tp);
    free(dataType);

    ADD_TYPE(tdef->name, tp);
    ADD_TYPE(";\n", tp);
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
        case STMT_LET:
            generateLocal(tp, (ASTLocal_T*) stmt->stmt);
            break;
        case STMT_EXPRESSION:
        {
            char* val = generateExpr(tp, ((ASTExprStmt_T*) stmt->stmt)->expr);
            ADD_IMPL(val, tp);
            free(val);
            ADD_IMPL(";\n", tp);
            break;
        }
        case STMT_IF:
            generateIf(tp, (ASTIf_T*) stmt->stmt);
            break;
        case STMT_LOOP:
            generateLoop(tp, (ASTLoop_T*) stmt->stmt);
            break;
        case STMT_MATCH:
            generateMatch(tp, (ASTMatch_T*) stmt->stmt);
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

static void generateLocal(transpiler_T* tp, ASTLocal_T* loc)
{
    char* dataType = generateType(tp, loc->dataType);
    ADD_IMPL(dataType, tp);
    free(dataType);

    ADD_IMPL(" ", tp);
    ADD_IMPL(loc->name, tp);

    if(loc->value)
    {
        ADD_IMPL("=", tp);
        char* value = generateExpr(tp, loc->value);
        ADD_IMPL(value, tp);
        free(value);
    }
    ADD_IMPL(";\n", tp);
}

static void generateIf(transpiler_T* tp, ASTIf_T* ifs)
{
    ADD_IMPL("if(", tp);
    char* cond = generateExpr(tp, ifs->condition);
    ADD_IMPL(cond, tp);
    free(cond);

    ADD_IMPL("){\n", tp);
    generateCompound(tp, ifs->ifBody);
    ADD_IMPL("}\n", tp);

    if(ifs->elseBody) {
        ADD_IMPL("else{\n", tp);
        generateCompound(tp, ifs->elseBody);
        ADD_IMPL("}\n", tp); 
    }
}

static void generateLoop(transpiler_T* tp, ASTLoop_T* loop)
{
    ADD_IMPL("while(", tp);
    char* cond = generateExpr(tp, loop->condition);
    ADD_IMPL(cond, tp);
    free(cond);

    ADD_IMPL("){\n", tp);
    generateCompound(tp, loop->body);
    ADD_IMPL("}\n", tp);
}

static void generateMatch(transpiler_T* tp, ASTMatch_T* match)
{
    ADD_IMPL("switch(", tp);
    char* cond = generateExpr(tp, match->condition);
    ADD_IMPL(cond, tp);
    free(cond);
    ADD_IMPL("){\n", tp);
    
    for(int i = 0; i < match->bodys->size; i++)
    {
        ASTCompound_T* body = (ASTCompound_T*) match->bodys->items[i];
        char* cond = generateExpr(tp, (ASTExpr_T*) match->cases->items[i]);

        ADD_IMPL("case ", tp);
        ADD_IMPL(cond, tp);
        free(cond);

        ADD_IMPL(":{\n", tp);
        generateCompound(tp, body);
        ADD_IMPL("}; break;\n", tp);   
    }

    if(match->defaultBody)
    {
        ADD_IMPL("default:{\n", tp);
        generateCompound(tp, match->defaultBody);
        ADD_IMPL("}; break;\n", tp); 
    }

    ADD_IMPL("}\n", tp);
}

static char* generateCallArgs(transpiler_T* tp, list_T* args)
{
    char* argsStr = malloc(sizeof(char));
    argsStr[0] = '\0';

    for(int i = 0; i < args->size; i++)
    {
        char* arg = generateExpr(tp, (ASTExpr_T*) args->items[i]);
        ADD_STR(arg, argsStr);
        free(arg);

        if(i < args->size - 1)
        {
           ADD_STR(",", argsStr);
        }
    }

    return argsStr;
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
        case EXPR_IDENTIFIER:
            return generateId(tp, (ASTIdentifier_T*) expr->expr);
        case EXPR_CALL:
        {
            const char* tmp = "%s(%s)";
            char* args = generateCallArgs(tp, ((ASTCall_T*) expr->expr)->args);
            char* callee = ((ASTCall_T*) expr->expr)->callee;
            char* call = calloc(strlen(tmp) 
                              + strlen(callee) 
                              + strlen(args) 
                              + 1, sizeof(char));
            sprintf(call, tmp, callee, args);
            free(args);
            return call;
        }
        case EXPR_INFIX:
            return generateInfixExpression(tp, (ASTInfix_T*) expr->expr);
        case EXPR_PREFIX:
            return generatePrefixExpression(tp, (ASTPrefix_T*) expr->expr);
        case EXPR_POSTFIX:
            return generatePostfixExpression(tp, (ASTPostfix_T*) expr->expr);
        case EXPR_INDEX:
            return generateIndexExpression(tp, (ASTIndex_T*) expr->expr);

        default:
            LOG_ERROR_F("Expression of type %d currently not support transpiling\n", expr->type);
            exit(1);
    }
}

static char* generateId(transpiler_T* tp, ASTIdentifier_T* id)
{
    char* idStr = calloc(sizeof(id->callee) + 1, sizeof(char));
    strcat(idStr, id->callee);

    if(id->childId)
    {
        char* childStr = generateId(tp, id->childId);
        if(id->isPtr) {
            ADD_STR("->", idStr);
        } else {
            ADD_STR(".", idStr);
        }
        ADD_STR(childStr, idStr);
        free(childStr);
    }

    return idStr;
}

static char* generateInfixExpression(transpiler_T* tp, ASTInfix_T* ifx)
{
    char* l = generateExpr(tp, ifx->left);
    char* r = generateExpr(tp, ifx->right);
    char* op;

    switch(ifx->op)
    {
        case OP_ADD:
            op = "+";
            break;
        case OP_SUB:
            op = "-";
            break;
        case OP_MULT:
            op = "*";
            break;
        case OP_DIV:
            op = "/";
            break;
        case OP_EQ:
            op = "==";
            break;
        case OP_NOT_EQ:
            op = "!=";
            break;
        case OP_GT:
            op = ">";
            break;
        case OP_LT:
            op = "<";
            break;
        case OP_GT_EQ:
            op = ">=";
            break;
        case OP_LT_EQ:
            op = "<=";
            break;
        case OP_ASSIGN:
            op = "=";
            break;

        default:
            LOG_ERROR_F("Infix operation of type %d currently not support transpiling\n", ifx->op);
            exit(1);
    }

    char* expr = calloc(strlen(l)
                      + strlen(r)
                      + strlen(op)
                      + strlen("%s%s%s")
                      + 1, sizeof(char));
    sprintf(expr, "%s%s%s", l, op, r);
    free(l);
    free(r);
    return expr;
}

static char* generatePrefixExpression(transpiler_T* tp, ASTPrefix_T* pfx)
{
    char* r = generateExpr(tp, pfx->right);
    char* op;

    switch(pfx->op)
    {
        case OP_NEGATE:
            op = "-";
            break;
        case OP_NOT:
            op = "!";
            break;
        case OP_REF:
            op = "&";
            break;
        case OP_DEREF:
            op = "*";
            break;
        case OP_BIT_NEG:
            op = "~";
            break;
        
        default:
            LOG_ERROR_F("Prefix operation of type %d currently not support transpiling\n", pfx->op);
            exit(1);
    }

    char* expr = calloc(strlen(r)
                      + strlen(op)
                      + strlen("%s%s")
                      + 1, sizeof(char));
    sprintf(expr, "%s%s", op, r);
    free(r);
    return expr;
}

static char* generatePostfixExpression(transpiler_T* tp, ASTPostfix_T* pfx)
{
    char* l = generateExpr(tp, pfx->left);
    char* op;

    switch(pfx->op)
    {
        case OP_INC:
            op = "++";
            break;
        case OP_DEC:
            op = "--";
            break;
        
        default:
            LOG_ERROR_F("Postfix operation of type %d currently not support transpiling\n", pfx->op);
            exit(1);
    }

    char* expr = calloc(strlen(l)
                      + strlen(op)
                      + strlen("%s%s")
                      + 1, sizeof(char));
    sprintf(expr, "%s%s", l, op);
    free(l);
    return expr;
}

static char* generateIndexExpression(transpiler_T* tp, ASTIndex_T* idx)
{
    const char* idxTmp = "%s[%s]";
    char* value = generateExpr(tp, idx->value);
    char* index = generateExpr(tp, idx->idx);
    char* idxStr = calloc(strlen(idxTmp)
                        + strlen(value)
                        + strlen(index)
                        + 1, sizeof(char));
    sprintf(idxStr, idxTmp, value, index);
    free(index);
    free(value);
    return idxStr;
}

static char* generateType(transpiler_T* tp, ASTType_T* type)
{
    switch(type->type)
    {
        case AST_VOID:
            return strdup("void");
        case AST_I32:
            return strdup("int32_t");
        case AST_I64:
            return strdup("ini64_t");
        case AST_U32:
            return strdup("u_int32_t");
        case AST_U64:
            return strdup("u_int64_t");
        case AST_F32:
            return strdup("float");
        case AST_F64:
            return strdup("double");
        case AST_CHAR:
            return strdup("char");
        case AST_BOOL:
            return strdup("u_int8_t");
        case AST_POINTER: {
            char* st = generateType(tp, type->subtype);
            st = realloc(st, (sizeof(st) + 2) * sizeof(char));
            strcat(st, "*");
            return st;
        }
        case AST_STRUCT: {
            return generateStructType(tp, (ASTStructType_T*) type->body);
        }
        case AST_ENUM: {
            return generateEnumType(tp, (ASTEnumType_T*) type->body);
        }
        case AST_TYPEDEF:
            return strdup(type->callee);
        default:
            LOG_ERROR_F("Transpiling of data type %d is currently not supported", type->type);
            exit(1);
    }
}


static char* generateStructType(transpiler_T* tp, ASTStructType_T* st)
{
    const char* structTmp = "struct {%s}";

    char* fieldsStr = malloc(sizeof(char));
    fieldsStr[0] = '\0';
    for(int i = 0; i < st->fieldNames->size; i++)
    {
        char* fType = generateType(tp, (ASTType_T*) st->fieldTypes->items[i]);
        ADD_STR(fType, fieldsStr);
        free(fType);
        ADD_STR(" ", fieldsStr);

        ADD_STR((char*) st->fieldNames->items[i], fieldsStr);
        ADD_STR(";", fieldsStr);
    }   
    char* structStr = calloc(strlen(fieldsStr)
                           + strlen(structTmp)
                           + 1, sizeof(char));
    sprintf(structStr, structTmp, fieldsStr);
    free(fieldsStr);
    return structStr;
}   

static char* generateEnumType(transpiler_T* tp, ASTEnumType_T* et)
{
    const char* enumTmp = "enum {%s}";

    char* fieldsStr = malloc(sizeof(char));
    fieldsStr[0] = '\0';
    for(int i = 0; i < et->fields->size; i++)
    {
        ADD_STR((char*) et->fields->items[i], fieldsStr);
        ADD_STR(",", fieldsStr);
    }
    char* enumStr = calloc(strlen(enumTmp)
                         + strlen(fieldsStr)
                         + 1, sizeof(char));
    sprintf(enumStr, enumTmp, fieldsStr);
    free(fieldsStr);
    return enumStr;
}