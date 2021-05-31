#include "ast.h"
#include "../io/log.h"
#include "types.h"

#include <string.h>

ASTProgram_T* init_ast_program(const char* mainFile)
{
    ASTProgram_T* p = malloc(sizeof(struct AST_PROGRAM_STRUCT));
    p->files = init_list(sizeof(struct AST_FILE_STRUCT*));
    p->main_file = strdup(mainFile);
    return p;
}

void free_ast_program(ASTProgram_T* p)
{
    free(p->main_file);
    
    for(int i = 0; i < p->files->size; i++)
        free_ast_file(p->files->items[i]);
    free_list(p->files);

    free(p);
}

ASTFile_T* init_ast_file(const char* filepath)
{
    ASTFile_T* r = calloc(1, sizeof(struct AST_FILE_STRUCT));

    r->filepath = strdup(filepath);
    r->functions = init_list(sizeof(struct AST_FUCTION_STRUCT*));
    r->globals = init_list(sizeof(struct AST_GLOBAL_STRUCT*));
    r->types = init_list(sizeof(struct AST_TYPEDEF_STRUCT*));

    return r;
}

void free_ast_file(ASTFile_T* r)
{
    free(r->filepath);

    for(int i = 0; i < r->functions->size; i++)
        free_ast_function((ASTFunction_T*) r->functions->items[i]);
    free_list(r->functions);

    for(int i = 0; i < r->globals->size; i++)
        free_ast_global((ASTGlobal_T*) r->globals->items[i]);
    free_list(r->globals);

    for(int i = 0; i < r->types->size; i++)
        free_ast_typedef((ASTTypedef_T*) r->types->items[i]);
    free_list(r->types);

    free(r);
}

ASTTypedef_T* init_ast_typedef(ASTType_T* type, const char* name, unsigned int line, unsigned int pos)
{
    ASTTypedef_T* t = calloc(1, sizeof(struct AST_TYPEDEF_STRUCT));
    t->data_type = type;
    t->name = strdup(name);

    t->line = line;
    t->pos = pos;
    return t;
}

void free_ast_typedef(ASTTypedef_T* t)
{
    free_ast_type(t->data_type);
    free(t->name);
    free(t);
}

ASTGlobal_T* init_ast_global(const char* name, ASTType_T* type, ASTExpr_T* value, unsigned int line, unsigned int pos)
{
    ASTGlobal_T* g = calloc(1, sizeof(struct AST_GLOBAL_STRUCT));
    g->name = strdup(name);
    g->type = type;
    g->value = value;

    g->is_mutable = false;

    g->line = line;
    g->pos = pos;
    return g;
}

void free_ast_global(ASTGlobal_T* g)
{
    free(g->name);

    free_ast_type(g->type);
    
    if(g->value != NULL)
        free_ast_expr(g->value);

    free(g);
}

ASTFunction_T* init_ast_function(const char* name, ASTType_T* return_type, ASTCompound_T* body, List_T* args, unsigned int line, unsigned int pos)
{
    ASTFunction_T* f = calloc(1, sizeof(struct AST_FUCTION_STRUCT));
    f->name = strdup(name);
    f->body = body;
    f->return_type = return_type;
    f->args = args;

    f->line = line;
    f->pos = pos;

    return f;
}

void free_ast_function(ASTFunction_T* f)
{
    for(int i = 0; i < f->args->size; i++)
        free_ast_argument(f->args->items[i]);
    free_list(f->args);

    free_ast_compound(f->body);
    free_ast_type(f->return_type);
    free(f->name);
    free(f);
}

ASTArgument_T* init_ast_argument(const char* name, ASTType_T* type)
{
    ASTArgument_T* a = calloc(1, sizeof(struct AST_ARGUMENT_STRUCT));
    a->name = strdup(name);
    a->type = type;
    a->offset = 0;
    return a;
}

void free_ast_argument(ASTArgument_T* a)
{
    free_ast_type(a->type);
    free(a->name);
    free(a);
}

ASTExprStmt_T* init_ast_expr_stmt(ASTExpr_T* expr)
{
    ASTExprStmt_T* e = calloc(1, sizeof(struct AST_EXPR_STMT_STRUCT));
    e->expr = expr;
    return e;
}

void free_ast_expr_stmt(ASTExprStmt_T* e)
{
    free_ast_expr(e->expr);
    free(e);
}

ASTMatch_T* init_ast_match(ASTExpr_T* condition, List_T* cases, List_T* bodys, ASTCompound_T* default_case)
{
    ASTMatch_T* m = calloc(1, sizeof(struct AST_MATCH_STRUCT));
    m->condition = condition;
    m->bodys = bodys;
    m->cases = cases;
    m->default_case = default_case;
    return m;
}

void free_ast_match(ASTMatch_T* m)
{
    free_ast_expr(m->condition);

    for(int i = 0; i < m->cases->size; i++)
        free_ast_expr(m->cases->items[i]);
    free_list(m->cases);

    for(int i = 0; i < m->bodys->size; i++)
        free_ast_expr(m->bodys->items[i]);
    free_list(m->bodys);

    if(m->default_case != NULL)
        free_ast_compound(m->default_case);
    
    free(m);
}

ASTLocal_T* init_ast_local(ASTType_T* type, ASTExpr_T* value, const char* name, unsigned int line, unsigned int pos)
{
    ASTLocal_T* l = calloc(1, sizeof(struct AST_LOCAL_STRUCT));
    l->type = type;
    l->value = value;
    l->name = strdup(name);
    l->line = line;
    l->pos = pos;
    l->isMutable = false;
    return l;
}

void free_ast_local(ASTLocal_T* l)
{
    free(l->name);
    
    if(l->value != NULL)
        free_ast_expr(l->value);
    free_ast_type(l->type);
    free(l);
}

ASTLoop_T* init_ast_loop(ASTExpr_T* condition, ASTCompound_T* body)
{
    ASTLoop_T* l = calloc(1, sizeof(struct AST_LOOP_STRUCT));
    l->body = body;
    l->condition = condition;
    return l;
}

void free_ast_loop(ASTLoop_T* l)
{
    free_ast_compound(l->body);
    free_ast_expr(l->condition);
    free(l);
}

ASTIf_T* init_ast_if(ASTExpr_T* condition, ASTCompound_T* if_body, ASTCompound_T* else_body)
{
    ASTIf_T* i = calloc(1, sizeof(struct AST_If_STRUCT));
    i->condition = condition;
    i->if_body = if_body;
    i->else_body = else_body;
    return i;
}

void free_ast_if(ASTIf_T* i)
{
    free_ast_compound(i->if_body);
    free_ast_expr(i->condition);

    if(i->else_body != NULL) 
        free_ast_compound(i->else_body);
    
    free(i);
}

ASTReturn_T* init_ast_return(ASTExpr_T* value)
{
    ASTReturn_T* r = calloc(1, sizeof(struct AST_RETURN_STRUCT));
    r->value = value;
    return r;
}

void free_ast_return(ASTReturn_T* r)
{
    free_ast_expr(r->value);
    free(r);
}

ASTStmt_T* init_ast_stmt(ASTStmtType_T type, void* stmt)
{
    ASTStmt_T* s = calloc(1, sizeof(struct AST_STATEMENT_STRUCT));
    s->stmt = stmt;
    s->type = type;
    return s;
}

void free_ast_stmt(ASTStmt_T* s)
{
    switch(s->type)
    {
        case STMT_EXPRESSION:
            free_ast_expr_stmt(s->stmt);
            break;
        case STMT_IF:
            free_ast_if(s->stmt);
            break;
        case STMT_LET:
            free_ast_local(s->stmt);
            break;
        case STMT_LOOP:
            free_ast_loop(s->stmt);
            break;
        case STMT_RETURN:
            free_ast_return(s->stmt);
            break;
        case STMT_MATCH:
            free_ast_match(s->stmt);
            break;
        default:
            LOG_ERROR_F("Undefined statement type %d\n", s->type);
            break;
    }
    free(s);
}

ASTArray_T* init_ast_array(List_T* exprs)
{
    ASTArray_T* a = calloc(1, sizeof(struct AST_ARRAY_LITERAL));
    a->exprs = exprs;
    a->len = a->exprs->size;
    return a;
}

void free_ast_array(ASTArray_T* a)
{
    for(int i = 0; i < a->len; i++)
        free_ast_expr(a->exprs->items[i]);
    free_list(a->exprs);
    free(a);
}

ASTBool_T* init_ast_bool(bool _bool)
{
    ASTBool_T* b = calloc(1, sizeof(struct AST_BOOL_LITERAL));
    b->_bool = _bool;
    return b;
}

void free_ast_bool(ASTBool_T* b)
{
    free(b);
}

ASTString_T* init_ast_string(const char* _string)
{
    ASTString_T* s = calloc(1, sizeof(struct AST_STRING_LITERAL));
    s->_string = strdup(_string);
    s->len = strlen(s->_string);
    return s;
}

void free_ast_string(ASTString_T* s)
{
    free(s->_string);
    free(s);
}

ASTChar_T* init_ast_char(char _char)
{
    ASTChar_T* c = calloc(1, sizeof(struct AST_CHAR_LITERAL));
    c->_char = _char;
    return c;
}

void free_ast_char(ASTChar_T* c)
{
    free(c);
}

ASTFloat_T* init_ast_float(float _float)
{
    ASTFloat_T* f = calloc(1, sizeof(struct AST_FLOAT_LITERAL));
    f->_float = _float;
    return f;
}

void free_ast_float(ASTFloat_T* f)
{
    free(f);
}

ASTInt_T* init_ast_int(int _int)
{
    ASTInt_T* i = calloc(1, sizeof(struct AST_INT_LITERAL));
    i->_int = _int;
    return i;
}

void free_ast_int(ASTInt_T* i)
{
    free(i);
}

ASTIdentifier_T* init_ast_identifier(const char* callee, ASTIdentifier_T* child)
{
    ASTIdentifier_T* i = calloc(1, sizeof(struct AST_IDENTIFIER_STRUCT));
    i->callee = strdup(callee);
    i->child_id = child;
    i->isPtr = false;
    return i;
}

void free_ast_identifier(ASTIdentifier_T* i)
{
    free(i->callee);
    if(i->child_id != NULL)
        free_ast_identifier(i->child_id);
    free(i);
}

ASTPostfix_T* init_ast_postfix(ASTPostfixOpType_T op, ASTExpr_T* left)
{
    ASTPostfix_T* p = calloc(1, sizeof(struct AST_POSTFIX_STRUCT));
    p->op = op;
    p->left = left;
    return p;
}

void free_ast_postfix(ASTPostfix_T* p)
{
    free_ast_expr(p->left);
    free(p);
}

ASTPrefix_T* init_ast_prefix(ASTPrefixOpType_T op, ASTExpr_T* right)
{
    ASTPrefix_T* p = calloc(1, sizeof(struct AST_PREFIX_STRUCT));
    p->op = op;
    p->right = right;
    return p;
}

void free_ast_prefix(ASTPrefix_T* p)
{
    free_ast_expr(p->right);
    free(p);
}

ASTInfix_T* init_ast_infix(ASTInfixOpType_T op, ASTExpr_T* right, ASTExpr_T* left)
{
    ASTInfix_T* i = calloc(1, sizeof(struct AST_INFIX_STRUCT));
    i->right = right; 
    i->left = left;
    i->op = op;
    return i;
}

void free_ast_infix(ASTInfix_T* i)
{
    free_ast_expr(i->left);

    if(i->right != NULL)
        free_ast_expr(i->right);

    free(i);
}

ASTCall_T* init_ast_call(const char* callee, List_T* args)
{
    ASTCall_T* c = calloc(1, sizeof(struct AST_CALL_STRUCT));
    c->callee = strdup(callee);
    c->args = args;
    return c;
}

void free_ast_call(ASTCall_T* c)
{   
    for(int i = 0; i < c->args->size; i++)
        free_ast_expr(c->args->items[i]);
        
    free_list(c->args);
    free(c->callee);
    free(c);
}

ASTIndex_T* init_ast_index(ASTExpr_T* value, ASTExpr_T* idx)
{
    ASTIndex_T* i = calloc(1, sizeof(struct AST_INDEX_STRUCT));
    i->idx = idx;
    i->value = value;
    return i;
}

void free_ast_index(ASTIndex_T* i)
{   
    free_ast_expr(i->idx);
    free_ast_expr(i->value);
    free(i);
}

ASTExpr_T* init_ast_expr(ASTType_T* data_type, ASTExprType_T type, void* ptr)
{
    ASTExpr_T* e = calloc(1, sizeof(struct AST_EXPRESSION_STRUCT));
    e->type = type;
    e->expr = ptr;
    e->data_type = data_type;
    return e;
}

void free_ast_expr(ASTExpr_T* e)
{
    switch(e->type) {
        case EXPR_PREFIX:
            free_ast_prefix(e->expr);
            break;
        case EXPR_INFIX:
            free_ast_infix(e->expr);
            break;
        case EXPR_POSTFIX:
            free_ast_postfix(e->expr);
            break;
        case EXPR_IDENTIFIER:
            free_ast_identifier(e->expr);
            break;
        case EXPR_CALL:
            free_ast_call(e->expr);
            break;
        case EXPR_INDEX:
            free_ast_index(e->expr);
            break;
        case EXPR_INT_LITERAL:
            free_ast_int(e->expr);
            break;
        case EXPR_CHAR_LITERAL:
            free_ast_char(e->expr);
            break;
        case EXPR_FLOAT_LITERAL:
            free_ast_float(e->expr);
            break;
        case EXPR_STRING_LITERAL:
            free_ast_string(e->expr);
            break;
        case EXPR_ARRAY_LITERAL:
            free_ast_array(e->expr);
            break;
        case EXPR_BOOL_LITERAL:
            free_ast_bool(e->expr);
            break;
        case EXPR_NIL:
            free_ast_nil(e->expr);
            break;
        case EXPR_STRUCT_LITERAL:
            free_ast_struct(e->expr);
            break;
        default:
            LOG_ERROR_F("Undefined epression ast type %d\n", e->type);
            break;
    }

    if(e->data_type != NULL)
        free_ast_type(e->data_type);

    free(e);
}

ASTCompound_T* init_ast_compound(List_T* stmts) {
    ASTCompound_T* c = calloc(1, sizeof(struct AST_COMPOUND_STRUCT));
    c->stmts = stmts;
    return c;
}

void free_ast_compound(ASTCompound_T* c)
{
    for(int i = 0; i < c->stmts->size; i++)
        free_ast_stmt(c->stmts->items[i]);
    free_list(c->stmts);

    free(c);
}

ASTType_T* init_ast_type(ASTDataType_T type, ASTType_T* subtype, void* body, char* callee, unsigned int line, unsigned int pos)
{
    ASTType_T* t = calloc(1, sizeof(struct AST_TYPE_STRUCT));
    t->type = type;
    t->subtype = subtype;
    t->body = body;
    t->free = true;
    t->line = line;
    t->pos = pos;
    t->size = 0;
    t->is_primitive = false;

    if(callee)
        t->callee = strdup(callee);

    return t;
}

void free_ast_type(ASTType_T* t)
{
    if(t->subtype)
        free_ast_type(t->subtype);

    if(t->free && t->is_primitive) 
    {
        if(t->type == AST_STRUCT)
            free_ast_struct_type(t->body);
        else if(t->type == AST_ENUM)
            free_ast_enum_type(t->body);

        if(t->callee != NULL)
            free(t->callee);

        free(t);
    }
}

ASTStructType_T* init_ast_struct_type(List_T* field_types, List_T* field_names)
{
    ASTStructType_T* s = calloc(1, sizeof(struct AST_STRUCT_TYPE_STRUCT));
    s->field_types = field_types;
    s->field_names = field_names;

    return s;
}

void free_ast_struct_type(ASTStructType_T* s)
{
    for(int i = 0; i < s->field_types->size; i++)
        free_ast_type(s->field_types->items[i]);
    free_list(s->field_types);

    for(int i = 0; i < s->field_types->size; i++)
        free(s->field_types->items[i]);
    free_list(s->field_types);

    free(s);
}

ASTEnumType_T* init_ast_enum_type(List_T* fields)
{
    ASTEnumType_T* e = calloc(1, sizeof(struct AST_ENUM_TYPE_STRUCT));
    e->fields = fields;
    return e;
}

void free_ast_enum_type(ASTEnumType_T* e)
{
    for(int i = 0; i < e->fields->size; i++)
        free(e->fields->items[i]);
    free_list(e->fields);

    free(e);
}

ASTNil_T* init_ast_nil()
{
    ASTNil_T* n = calloc(1, sizeof(struct AST_NIL_LITERAL));
    return n;
}

void free_ast_nil(ASTNil_T* n)
{
    free(n);
}

ASTStruct_T* init_ast_struct(List_T* exprs, List_T* fields)
{
    ASTStruct_T* s = calloc(1, sizeof(struct AST_STRUCT_LITERAL));
    s->exprs = exprs;
    s->fields = fields;
    return s;
}

void free_ast_struct(ASTStruct_T* s)
{
    for(int i = 0; i < s->fields->size; i++)
        free(s->fields->items[i]);
    free_list(s->fields);

    for(int i = 0; i < s->exprs->size; i++)
        free_ast_expr(s->exprs->items[i]);
    free_list(s->exprs);

    free(s);
}