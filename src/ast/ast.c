#include "ast.h"
#include "../io/log.h"

#include <string.h>

ASTRoot_T* initASTRoot(const char* filepath)
{
    ASTRoot_T* r = calloc(1, sizeof(struct AST_ROOT_STRUCT));

    r->filepath = strdup(filepath);
    r->functions = initList(sizeof(struct AST_FUCTION_STRUCT*));
    r->globals = initList(sizeof(struct AST_GLOBAL_STRUCT*));
    r->imports = initList(sizeof(struct AST_IMPORT_STRUCT*));
    r->types = initList(sizeof(struct AST_TYPEDEF_STRUCT*));

    return r;
}

void freeASTRoot(ASTRoot_T* r)
{
    free(r->filepath);

    for(int i = 0; i < r->functions->size; i++)
        freeASTFunction((ASTFunction_T*) r->functions->items[i]);
    freeList(r->functions);

    for(int i = 0; i < r->globals->size; i++)
        freeASTGlobal((ASTGlobal_T*) r->globals->items[i]);
    freeList(r->globals);

    for(int i = 0; i < r->imports->size; i++)
        freeASTImport((ASTImport_T*) r->imports->items[i]);
    freeList(r->imports);

    for(int i = 0; i < r->types->size; i++)
        freeASTTypedef((ASTTypedef_T*) r->types->items[i]);
    freeList(r->types);

    free(r);
}

ASTImport_T* initASTImport(const char* file)
{
    ASTImport_T* i = calloc(1, sizeof(struct AST_IMPORT_STRUCT));
    i->includeFile = strdup(file);
    return i;
}

void freeASTImport(ASTImport_T* i)
{
    free(i->includeFile);
    free(i);
}

ASTTypedef_T* initASTTypedef(ASTType_T* type, const char* name)
{
    ASTTypedef_T* t = calloc(1, sizeof(struct AST_TYPEDEF_STRUCT));
    t->dataType = type;
    t->name = strdup(name);
    return t;
}

void freeASTTypedef(ASTTypedef_T* t)
{
    freeASTType(t->dataType);
    free(t->name);
    free(t);
}

ASTGlobal_T* initASTGlobal(const char* name, ASTType_T* type, ASTExpr_T* value)
{
    ASTGlobal_T* g = calloc(1, sizeof(struct AST_GLOBAL_STRUCT));
    g->name = strdup(name);
    g->type = type;
    g->value = value;
    return g;
}

void freeASTGlobal(ASTGlobal_T* g)
{
    free(g->name);
    freeASTType(g->type);
    
    if(g->value != NULL)
        freeASTExpr(g->value);

    free(g);
}

ASTFunction_T* initASTFunction(const char* name, ASTType_T* returnType, ASTCompound_T* body, list_T* args)
{
    ASTFunction_T* f = calloc(1, sizeof(struct AST_FUCTION_STRUCT));
    f->name = strdup(name);
    f->body = body;
    f->returnType = returnType;
    f->args = args;
    return f;
}

void freeASTFunction(ASTFunction_T* f)
{
    for(int i = 0; i < f->args->size; i++)
        freeASTArgument(f->args->items[i]);
    freeList(f->args);

    freeASTCompound(f->body);
    freeASTType(f->returnType);
    free(f->name);
    free(f);
}

ASTArgument_T* initASTArgument(const char* name, ASTType_T* dataType)
{
    ASTArgument_T* a = calloc(1, sizeof(struct AST_ARGUMENT_STRUCT));
    a->name = strdup(name);
    a->dataType = dataType;
    return a;
}

void freeASTArgument(ASTArgument_T* a)
{
    freeASTType(a->dataType);
    free(a->name);
    free(a);
}

ASTExprStmt_T* initASTExprStmt(ASTExpr_T* expr)
{
    ASTExprStmt_T* e = calloc(1, sizeof(struct AST_EXPR_STMT_STRUCT));
    e->expr = expr;
    return e;
}

void freeASTExprStmt(ASTExprStmt_T* e)
{
    freeASTExpr(e->expr);
    free(e);
}

ASTMatch_T* initASTMatch(list_T* conditions, list_T* bodys, ASTCompound_T* defaultBody)
{
    ASTMatch_T* m = calloc(1, sizeof(struct AST_MATCH_STRUCT));
    m->bodys = bodys;
    m->conditions = conditions;
    m->defaultBody = defaultBody;
    return m;
}

void freeASTMatch(ASTMatch_T* m)
{
    for(int i = 0; i < m->conditions->size; i++)
        freeASTExpr(m->conditions->items[i]);
    freeList(m->conditions);

    for(int i = 0; i < m->bodys->size; i++)
        freeASTCompound(m->bodys->items[i]);
    freeList(m->bodys);

    if(m->defaultBody != NULL)
        free(m->defaultBody);
    
    free(m);
}

ASTLocal_T* initASTLocal(ASTType_T* dataType, ASTExpr_T* value, const char* name)
{
    ASTLocal_T* l = calloc(1, sizeof(struct AST_LOCAL_STRUCT));
    l->dataType = dataType;
    l->value = value;
    l->name = strdup(name);
    return l;
}

void freeASTLocal(ASTLocal_T* l)
{
    free(l->name);
    
    if(l->value != NULL)
        freeASTExpr(l->value);

    freeASTType(l->dataType);
    free(l);
}

ASTLoop_T* initASTLoop(ASTExpr_T* condition, ASTCompound_T* body)
{
    ASTLoop_T* l = calloc(1, sizeof(struct AST_LOOP_STRUCT));
    l->body = body;
    l->condition = condition;
    return l;
}

void freeASTLoop(ASTLoop_T* l)
{
    freeASTCompound(l->body);
    freeASTExpr(l->condition);
    free(l);
}

ASTIf_T* initASTIf(ASTExpr_T* condition, ASTCompound_T* ifBody, ASTCompound_T* elseBody)
{
    ASTIf_T* i = calloc(1, sizeof(struct AST_If_STRUCT));
    i->condition = condition;
    i->ifBody = ifBody;
    i->elseBody = elseBody;
    return i;
}

void freeASTIf(ASTIf_T* i)
{
    freeASTCompound(i->ifBody);
    freeASTExpr(i->condition);

    if(i->elseBody != NULL) 
        freeASTCompound(i->elseBody);
    
    free(i);
}

ASTReturn_T* initASTReturn(ASTType_T* type, ASTExpr_T* value)
{
    ASTReturn_T* r = calloc(1, sizeof(struct AST_RETURN_STRUCT));
    r->value = value;
    r->returnType = type;
    return r;
}

void freeASTReturn(ASTReturn_T* r)
{
    freeASTExpr(r->value);
    freeASTType(r->returnType);

    free(r);
}

ASTStmt_T* initASTStmt(ASTStmtType_T type, void* stmt)
{
    ASTStmt_T* s = calloc(1, sizeof(struct AST_STATEMENT_STRUCT));
    s->stmt = stmt;
    s->type = type;
    return s;
}

void freeASTStmt(ASTStmt_T* s)
{
    switch(s->type)
    {
        case STMT_EXPRESSION:
            freeASTExprStmt(s->stmt);
            break;
        case STMT_IF:
            freeASTIf(s->stmt);
            break;
        case STMT_LET:
            freeASTLocal(s->stmt);
            break;
        case STMT_LOOP:
            freeASTLoop(s->stmt);
            break;
        case STMT_RETURN:
            freeASTReturn(s->stmt);
            break;
        case STMT_MATCH:
            freeASTMatch(s->stmt);
            break;
        default:
            LOG_ERROR_F("Undefined statement type %d\n", s->type);
            break;
    }
    free(s);
}

ASTArray_T* initASTArray(list_T* exprs)
{
    ASTArray_T* a = calloc(1, sizeof(struct AST_ARRAY_LITERAL));
    a->exprs = exprs;
    a->len = a->exprs->size;
    return a;
}

void freeASTArray(ASTArray_T* a)
{
    for(int i = 0; i < a->len; i++)
        freeASTExpr(a->exprs->items[i]);
    free(a);
}

ASTBool_T* initASTBool(bool _bool)
{
    ASTBool_T* b = calloc(1, sizeof(struct AST_BOOL_LITERAL));
    b->_bool = _bool;
    return b;
}

void freeASTBool(ASTBool_T* b)
{
    free(b);
}

ASTString_T* initASTString(const char* _string)
{
    ASTString_T* s = calloc(1, sizeof(struct AST_STRING_LITERAL));
    s->_string = strdup(_string);
    s->len = strlen(s->_string);
    return s;
}

void freeASTString(ASTString_T* s)
{
    free(s->_string);
    free(s);
}

ASTChar_T* initASTChar(char _char)
{
    ASTChar_T* c = calloc(1, sizeof(struct AST_CHAR_LITERAL));
    c->_char = _char;
    return c;
}

void freeASTChar(ASTChar_T* c)
{
    free(c);
}

ASTFloat_T* initASTFloat(float _float)
{
    ASTFloat_T* f = calloc(1, sizeof(struct AST_FLOAT_LITERAL));
    f->_float = _float;
    return f;
}

void freeASTFloat(ASTFloat_T* f)
{
    free(f);
}

ASTInt_T* initASTInt(int _int)
{
    ASTInt_T* i = calloc(1, sizeof(struct AST_INT_LITERAL));
    i->_int = _int;
    return i;
}

void freeASTInt(ASTInt_T* i)
{
    free(i);
}

ASTIdentifer_T* initASTIdentifier(const char* callee)
{
    ASTIdentifer_T* i = calloc(1, sizeof(struct AST_IDENTIFIER_STRUCT));
    i->callee = strdup(callee);
    return i;
}

void freeASTIdentifier(ASTIdentifer_T* i)
{
    free(i->callee);
    free(i);
}

ASTPrefix_T* initASTPrefix(ASTPrefixOpType_T op, ASTExpr_T* right)
{
    ASTPrefix_T* p = calloc(1, sizeof(struct AST_PREFIX_STRUCT));
    p->op = op;
    p->right = right;
    return p;
}

void freeASTPrefix(ASTPrefix_T* p)
{
    freeASTExpr(p->right);
    free(p);
}

ASTInfix_T* initASTInfix(ASTInfixOpType_T op, ASTExpr_T* right, ASTExpr_T* left)
{
    ASTInfix_T* i = calloc(1, sizeof(struct AST_INFIX_STRUCT));
    i->right = right; 
    i->left = left;
    i->op = op;
    return i;
}

void freeASTInfix(ASTInfix_T* i)
{
    freeASTExpr(i->left);

    if(i->right != NULL)
        freeASTExpr(i->right);

    free(i);
}

ASTExpr_T* initASTExpr(ASTType_T* dataType, ASTExprType_T type, void* ptr)
{
    ASTExpr_T* e = calloc(1, sizeof(struct AST_EXPRESSION_STRUCT));
    e->type = type;
    e->expr = ptr;
    e->dataType = dataType;
    return e;
}

void freeASTExpr(ASTExpr_T* e)
{
    freeASTType(e->dataType);

    switch(e->type) {
        case EXPR_PREFIX:
            freeASTPrefix(e->expr);
            break;
        case EXPR_INFIX:
            freeASTInfix(e->expr);
            break;
        case EXPR_IDENTIFIER:
            freeASTIdentifier(e->expr);
            break;
        case EXPR_INT_LITERAL:
            freeASTInt(e->expr);
            break;
        case EXPR_CHAR_LITERAL:
            freeASTChar(e->expr);
            break;
        case EXPR_FLOAT_LITERAL:
            freeASTFloat(e->expr);
            break;
        case EXPR_STRING_LITERAL:
            freeASTString(e->expr);
            break;
        case EXPR_ARRAY_LITERAL:
            freeASTArray(e->expr);
            break;
        case EXPR_BOOL_LITERAL:
            freeASTBool(e->expr);
            break;
        default:
            LOG_ERROR_F("Undefined epression ast type %d\n", e->type);
            break;
    }

    free(e);
}

ASTCompound_T* initASTCompound(list_T* stmts)
{
    ASTCompound_T* c = calloc(1, sizeof(struct AST_COMPOUND_STRUCT));
    c->stmts = stmts;
    return c;
}

void freeASTCompound(ASTCompound_T* c)
{
    for(int i = 0; i < c->stmts->size; i++)
        freeASTStmt(c->stmts->items[i]);
    freeList(c->stmts);

    free(c);
}

ASTType_T* initASTType(ASTDataType_T type, ASTType_T* subtype)
{
    ASTType_T* t = calloc(1, sizeof(struct AST_TYPE_STRUCT));
    t->type = type;
    t->subtype = subtype;

    return t;
}

void freeASTType(ASTType_T* t)
{
    if(t->subtype != NULL)
        freeASTType(t->subtype);
    free(t);
}