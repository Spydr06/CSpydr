#include "parser.h"
#include "../io/log.h"
#include "../io/io.h"

#include "../ast/types.h"
#include "../platform/platform_bindings.h"

#include <string.h>
#include <stdio.h>

#ifdef __linux__
    #include <libgen.h>
#endif

#include <unistd.h>

#define SYNTAX_ERROR(parser, msg) throw_syntax_error(parser->eh, msg, parser->tok->line, parser->tok->pos);

/////////////////////////////////
// expression parsing settings //
/////////////////////////////////

static ASTNode_T* parse_id(Parser_T* p);
static ASTNode_T* parse_int_lit(Parser_T* p);
static ASTNode_T* parse_float_lit(Parser_T* p);
static ASTNode_T* parse_char_lit(Parser_T* p);
static ASTNode_T* parse_bool_lit(Parser_T* p);
static ASTNode_T* parse_str_lit(Parser_T* p);
static ASTNode_T* parse_nil_lit(Parser_T* p);
static ASTNode_T* parse_closure(Parser_T* p);

static ASTNode_T* parse_array_lit(Parser_T* p);
static ASTNode_T* parse_struct_lit(Parser_T* p);

static ASTNode_T* parse_unary(Parser_T* p);
static ASTNode_T* parse_num_op(Parser_T* p, ASTNode_T* left);
static ASTNode_T* parse_bool_op(Parser_T* p, ASTNode_T* left);
static ASTNode_T* parse_assignment(Parser_T* p, ASTNode_T* left);
static ASTNode_T* parse_postfix(Parser_T* p, ASTNode_T* left);
static ASTNode_T* parse_index(Parser_T* p, ASTNode_T* left);
static ASTNode_T* parse_member(Parser_T* p, ASTNode_T* left);

static ASTNode_T* parse_call(Parser_T* p, ASTNode_T* left);

static struct { prefix_parse_fn pfn; infix_parse_fn ifn; Precedence_T prec; } expr_parse_fns[TOKEN_EOF + 1] = {
    [TOKEN_ID]       = {parse_id, NULL, LOWEST},
    [TOKEN_INT]      = {parse_int_lit, NULL, LOWEST},
    [TOKEN_FLOAT]    = {parse_float_lit, NULL, LOWEST},
    [TOKEN_NIL]      = {parse_nil_lit, NULL, LOWEST},
    [TOKEN_TRUE]     = {parse_bool_lit, NULL, LOWEST},
    [TOKEN_FALSE]    = {parse_bool_lit, NULL, LOWEST},
    [TOKEN_CHAR]     = {parse_char_lit, NULL, LOWEST},
    [TOKEN_STRING]   = {parse_str_lit, NULL, LOWEST},
    [TOKEN_BANG]     = {parse_unary, NULL, LOWEST},
    [TOKEN_MINUS]    = {parse_unary, parse_num_op, SUM},
    [TOKEN_LPAREN]   = {parse_closure, parse_call, CALL}, 
    [TOKEN_LBRACKET] = {parse_array_lit, parse_index, INDEX},   
    [TOKEN_LBRACE]   = {parse_struct_lit, NULL, LOWEST}, 
    [TOKEN_STAR]     = {parse_unary, parse_num_op, PRODUCT},
    [TOKEN_REF]      = {parse_unary, NULL, LOWEST},
    [TOKEN_TILDE]    = {parse_unary, NULL, LOWEST},
    [TOKEN_PLUS]     = {NULL, parse_num_op, SUM},    
    [TOKEN_SLASH]    = {NULL, parse_num_op, PRODUCT},    
    [TOKEN_EQ]       = {NULL, parse_bool_op, EQUALS}, 
    [TOKEN_NOT_EQ]   = {NULL, parse_bool_op, EQUALS},     
    [TOKEN_GT]       = {NULL, parse_bool_op, LTGT}, 
    [TOKEN_GT_EQ]    = {NULL, parse_bool_op, LTGT},    
    [TOKEN_LT]       = {NULL, parse_bool_op, LTGT}, 
    [TOKEN_LT_EQ]    = {NULL, parse_bool_op, LTGT},           
    [TOKEN_INC]      = {NULL, parse_postfix, POSTFIX},  
    [TOKEN_DEC]      = {NULL, parse_postfix, POSTFIX},  
    [TOKEN_ASSIGN]   = {NULL, parse_assignment, ASSIGN},     
    [TOKEN_ADD]      = {NULL, parse_assignment, ASSIGN},  
    [TOKEN_SUB]      = {NULL, parse_assignment, ASSIGN},  
    [TOKEN_DIV]      = {NULL, parse_assignment, ASSIGN},  
    [TOKEN_MULT]     = {NULL, parse_assignment, ASSIGN},   
    [TOKEN_DOT]      = {NULL, parse_member, MEMBER},  
}; 

static ASTNodeKind_T unary_ops[TOKEN_EOF + 1] = {
    [TOKEN_MINUS] = ND_NEG,
    [TOKEN_BANG]  = ND_NOT,
    [TOKEN_TILDE] = ND_BIT_NEG,
    [TOKEN_REF]   = ND_REF,
    [TOKEN_STAR]  = ND_DEREF
};

static ASTNodeKind_T infix_ops[TOKEN_EOF + 1] = {
    [TOKEN_MINUS] = ND_SUB,
    [TOKEN_PLUS]  = ND_ADD,
    [TOKEN_STAR]  = ND_MUL,
    [TOKEN_SLASH] = ND_DIV,

    [TOKEN_EQ]     = ND_EQ,
    [TOKEN_NOT_EQ] = ND_NE,
    [TOKEN_GT]     = ND_GT,
    [TOKEN_GT_EQ]  = ND_GE,
    [TOKEN_LT]     = ND_LT,
    [TOKEN_LT_EQ]  = ND_LE,

    [TOKEN_ASSIGN] = ND_ASSIGN,
    [TOKEN_ADD]    = ND_ADD,    // is still an assignment!
    [TOKEN_SUB]    = ND_SUB,    // is still an assignment!
    [TOKEN_MULT]   = ND_MUL,    // is still an assignment!
    [TOKEN_DIV]    = ND_DIV,    // is still an assignment!

    [TOKEN_DOT]    = ND_MEMBER,

    [TOKEN_INC] = ND_INC,   // technically postfix operators, but get treated like infix ops internally
    [TOKEN_DEC] = ND_DEC    // technically postfix operators, but get treated like infix ops internally
};

static inline prefix_parse_fn get_prefix_parse_fn(TokenType_T tt)
{
    return expr_parse_fns[tt].pfn;
}

static inline infix_parse_fn get_infix_parse_fn(TokenType_T tt)
{
    return expr_parse_fns[tt].ifn;
}

static inline Precedence_T get_precedence(TokenType_T tt)
{
    return expr_parse_fns[tt].prec;
}

/////////////////////////////////
// helperfunctions             //
/////////////////////////////////

Parser_T* init_parser(Lexer_T* lexer)
{
    Parser_T* parser = calloc(1, sizeof(struct PARSER_STRUCT));
    parser->lexer = lexer;
    parser->tok = lexer_next_token(parser->lexer);
    parser->imports = init_list(sizeof(char*));

    parser->current_block = NULL;
    return parser;
}

void free_parser(Parser_T* p)
{
    free_token(p->tok);

    for(int i = 0; i < p->imports->size; i++)
        free((char*) p->imports->items[i]);
    free_list(p->imports);

    free(p);
}

static inline Token_T* parser_advance(Parser_T* p)
{
    free_token(p->tok);
    p->tok = lexer_next_token(p->lexer);
    return p->tok;
}

static inline bool tok_is(Parser_T* p, TokenType_T type)
{
    return p->tok->type == type;
}

static inline bool streq(char* s1, char* s2)
{
    return strcmp(s1, s2) == 0;
}

Token_T* parser_consume(Parser_T* p, TokenType_T type, const char* msg)
{
    if(!tok_is(p, type))
        throw_error(ERR_SYNTAX_ERROR, p->tok, "unexpected token `%s`, %s", msg);

    return parser_advance(p);
}

static inline bool is_editable(ASTNodeKind_T n)
{
    return n == ND_ID || n == ND_MEMBER || n == ND_INDEX;
}

static inline bool is_executable(ASTNodeKind_T n)
{
    return n == ND_CALL || n == ND_ASSIGN || n == ND_INC || n == ND_DEC;
}

static bool is_already_imported(ASTProg_T* prog, char* path)
{
    for(int i = 0; i < prog->imports->size; i++)
        if(streq(prog->imports->items[i], path))
            return true;
    return false;
}

/////////////////////////////////
// Parser                      //
/////////////////////////////////

static void parse_import(Parser_T* p, ASTProg_T* prog);
static ASTObj_T* parse_typedef(Parser_T* p);
static ASTObj_T* parse_fn(Parser_T* p);
static ASTObj_T* parse_global(Parser_T* p);

ASTProg_T* parse_file(List_T* imports, SrcFile_T* src, bool is_silent)
{
    Lexer_T* lex = init_lexer(src);
    Parser_T* p = init_parser(lex);

    if(!is_silent)
    {
        LOG_OK_F(COLOR_BOLD_GREEN "  Compiling " COLOR_RESET " %s\n", src->path);
    }

    ASTProg_T* prog = init_ast_prog(src->path, NULL, imports);

    while(!tok_is(p, TOKEN_EOF))
    {
        switch(p->tok->type)
        {
            case TOKEN_IMPORT:
                parse_import(p, prog);
                break;
            case TOKEN_TYPE:
                list_push(prog->objs, parse_typedef(p));
                break;
            case TOKEN_LET:
                list_push(prog->objs, parse_global(p));
                break;
            case TOKEN_FN:
                list_push(prog->objs, parse_fn(p));
                break;
            default:
                throw_error(ERR_SYNTAX_ERROR, p->tok, "unexpected token `%s`, expect [import, type, let, fn]", p->tok->value);
        }
    }

    free_parser(p);
    free_lexer(lex);

    return prog;
}

static char* get_full_import_path(Parser_T* p, char* origin, Token_T* import_file)
{
    // first get the full directory of the origin
    char* abs_path = get_absolute_path(origin);
    char* full_path = get_path_from_file(abs_path);

    // construct the imported file onto it
    const char* template = "%s" DIRECTORY_DELIMS "%s";
    char* full_import_path = calloc(strlen(template) + strlen(full_path) + strlen(import_file->value) + 1, sizeof(char));
    sprintf(full_import_path, template, full_path, import_file->value);

    if(!file_exists(full_import_path))
    {
        throw_error(ERR_SYNTAX_ERROR, p->tok, "Error reading imported file \"%s\", no such file or directory", import_file->value);
    }

    return full_import_path;
}

static void parse_import(Parser_T* p, ASTProg_T* prog)
{
    parser_consume(p, TOKEN_IMPORT, "expect `import` keyword to import a file");
    
    char* path = get_full_import_path(p, (char*) prog->main_file_path, p->tok);

    if(!is_already_imported(prog, path))
        list_push(prog->imports, strdup(path));

    parser_consume(p, TOKEN_STRING, "expect \"<import file>\" after import keyword");
    parser_consume(p, TOKEN_SEMICOLON, "expect `;` after import");
}

static ASTNode_T* parse_expr(Parser_T* p, Precedence_T prec, TokenType_T end_tok);
static ASTType_T* parse_type(Parser_T* p);

static ASTType_T* parse_struct_type(Parser_T* p)
{
    ASTType_T* struct_type = init_ast_type(TY_STRUCT, p->tok);
    parser_consume(p, TOKEN_STRUCT, "expect `struct` keyword for struct type");
    parser_consume(p, TOKEN_LBRACE, "expect `{` after struct keyword");
    struct_type->members = init_list(sizeof(struct AST_NODE_STRUCT*));

    while(!tok_is(p, TOKEN_RBRACE) && !tok_is(p, TOKEN_EOF))
    {
        ASTNode_T* member = init_ast_node(ND_STRUCT_MEMBER, p->tok);
        member->callee = member->tok->value;
        parser_consume(p, TOKEN_ID, "expect struct member name");
        parser_consume(p, TOKEN_COLON, "expect `:` after struct member name");
        member->data_type = parse_type(p);

        list_push(struct_type->members, member);

        if(!tok_is(p, TOKEN_RBRACE))
            parser_consume(p, TOKEN_COMMA, "expect `,` between struct members");
    }

    parser_consume(p, TOKEN_RBRACE, "expect `}` after struct members");
    return struct_type;
}

static ASTType_T* parse_enum_type(Parser_T* p)
{
    ASTType_T* enum_type = init_ast_type(TY_ENUM, p->tok);
    parser_consume(p, TOKEN_ENUM, "expect `enum` keyword for enum type");
    parser_consume(p, TOKEN_LBRACE, "expect `{` after enum keyword");
    enum_type->members = init_list(sizeof(struct AST_NODE_STRUCT*));

    for(int i = 0; !tok_is(p, TOKEN_RBRACE) && !tok_is(p, TOKEN_EOF); i++)
    {
        ASTNode_T* member = init_ast_node(ND_ENUM_MEMBER, p->tok);
        member->callee = member->tok->value;
        member->int_val = i;
        parser_consume(p, TOKEN_ID, "expect enum member name");

        list_push(enum_type->members, member);

        if(!tok_is(p, TOKEN_RBRACE))
            parser_consume(p, TOKEN_COMMA, "expect `,` between enum members");
    }

    parser_consume(p, TOKEN_RBRACE, "expect `}` after enum members");
    return enum_type;
}

static ASTType_T* parse_type(Parser_T* p)
{
    ASTType_T* type = get_primitive_type(p->tok->value);
    if(type)
    {
        parser_advance(p);
        return type;
    }

    switch(p->tok->type)
    {
        case TOKEN_STRUCT:
            type = parse_struct_type(p);
            break;
        case TOKEN_ENUM:
            type = parse_enum_type(p);
            break;
        case TOKEN_STAR:
            type = init_ast_type(TY_PTR, p->tok);
            parser_advance(p);
            type->base = parse_type(p);
            break;
        case TOKEN_LBRACKET:
            type = init_ast_type(TY_ARR, p->tok);
            parser_advance(p);
            if(!tok_is(p, TOKEN_RBRACKET))
                type->num_indices = parse_expr(p, LOWEST, TOKEN_RBRACKET);
            parser_consume(p, TOKEN_RBRACKET, "expect `]` after array type");
            type->base = parse_type(p);
            break;
        default:
            type = init_ast_type(TY_UNDEF, p->tok);
            type->callee = type->tok->value;
            parser_consume(p, TOKEN_ID, "expect type or typedef");
            break;
    }

    return type;
}

static ASTObj_T* parse_typedef(Parser_T* p)
{
    ASTObj_T* tydef = init_ast_obj(OBJ_TYPEDEF, p->tok);
    parser_consume(p, TOKEN_TYPE, "expect `type` keyword for typedef");
    
    tydef->callee = strdup(p->tok->value);
    parser_consume(p, TOKEN_ID, "expect type name");
    parser_consume(p, TOKEN_COLON, "expect `:` after type name");

    tydef->data_type = parse_type(p);

    parser_consume(p, TOKEN_SEMICOLON, "expect `;` after typedef type");
    return tydef;
}

static List_T* parse_argument_list(Parser_T* p, TokenType_T end_tok)
{
    List_T* arg_list = init_list(sizeof(ASTObj_T*));

    while(p->tok->type != end_tok)
    {
        ASTObj_T* arg = init_ast_obj(OBJ_FN_ARG, p->tok);
        arg->callee = strdup(p->tok->value);
        parser_consume(p, TOKEN_ID, "expect argument name");
        parser_consume(p, TOKEN_COLON, "expect `:` after argument name");

        arg->data_type = parse_type(p);
        list_push(arg_list, arg);

        if(p->tok->type != end_tok)
            parser_consume(p, TOKEN_COMMA, "expect `,` between arguments");
    }

    return arg_list;
}

static ASTNode_T* parse_stmt(Parser_T* p);

static ASTObj_T* parse_fn(Parser_T* p)
{
    ASTObj_T* fn = init_ast_obj(OBJ_FUNCTION, p->tok);
    parser_consume(p, TOKEN_FN, "expect `fn` keyword for a function definition");

    fn->callee = strdup(p->tok->value);
    parser_consume(p, TOKEN_ID, "expect function name");

    parser_consume(p, TOKEN_LPAREN, "expect `(` after function name");

    fn->args = parse_argument_list(p, TOKEN_RPAREN);

    parser_consume(p, TOKEN_RPAREN, "expect `)` after function arguments");

    if(tok_is(p, TOKEN_COLON))
    {
        parser_advance(p);
        fn->return_type = parse_type(p);
    } else
        fn->return_type = primitives[TY_VOID];

    fn->body = parse_stmt(p);

    return fn;
}

static ASTObj_T* parse_global(Parser_T* p)
{
    ASTObj_T* global = init_ast_obj(OBJ_GLOBAL, p->tok);
    parser_consume(p, TOKEN_LET, "expect `let` keyword for variable definition");
    
    global->callee = strdup(p->tok->value);
    parser_consume(p, TOKEN_ID, "expect variable name");
    parser_consume(p, TOKEN_COLON, "expect `:` after variable name");

    global->data_type = parse_type(p);

    if(tok_is(p, TOKEN_ASSIGN))
    {
        parser_advance(p);
        global->value = parse_expr(p, LOWEST, TOKEN_SEMICOLON);
        if(!global->value->is_constant)
            throw_error(ERR_UNDEFINED, p->tok, "assigned value unknown at compile-time");
    }
    
    parser_consume(p, TOKEN_SEMICOLON, "expect `;` after variable declaration");

    return global;
}

/////////////////////////////////
// Statement Parser            //
/////////////////////////////////

static ASTNode_T* parse_block(Parser_T* p)
{
    ASTNode_T* block = init_ast_node(ND_BLOCK, p->tok);
    block->locals = init_list(sizeof(struct AST_OBJ_STRUCT*));
    block->stmts = init_list(sizeof(struct AST_NODE_STRUCT*));

    parser_consume(p, TOKEN_LBRACE, "expect `{` at the beginning of a block statement");

    ASTNode_T* prev_block = p->current_block;
    p->current_block = block;
    while(p->tok->type != TOKEN_RBRACE)
        list_push(block->stmts, parse_stmt(p));
    p->current_block = prev_block;

    parser_consume(p, TOKEN_RBRACE, "expect `}` at the end of a block statement");

    return block;
}

static ASTNode_T* parse_return(Parser_T* p)
{
    ASTNode_T* ret = init_ast_node(ND_RETURN, p->tok);

    parser_consume(p, TOKEN_RETURN, "expect `ret` or `<-` to return");

    if(!tok_is(p, TOKEN_SEMICOLON))
        ret->return_val = parse_expr(p, LOWEST, TOKEN_SEMICOLON);
    parser_consume(p, TOKEN_SEMICOLON, "expect `;` after return statement");

    return ret;
}

static ASTNode_T* parse_if(Parser_T* p)
{
    ASTNode_T* if_stmt = init_ast_node(ND_IF, p->tok);

    parser_consume(p, TOKEN_IF, "expect `if` keyword for an if statement");

    if_stmt->condition = parse_expr(p, LOWEST, TOKEN_EOF);
    if_stmt->if_branch = parse_stmt(p);

    if(tok_is(p, TOKEN_ELSE))
    {
        parser_advance(p);
        if_stmt->else_branch = parse_stmt(p);
    }

    return if_stmt;
}

static ASTNode_T* parse_loop(Parser_T* p)
{
    ASTNode_T* loop = init_ast_node(ND_LOOP, p->tok);

    parser_consume(p, TOKEN_LOOP, "expect `loop` keyword for a loop");

    loop->condition = parse_expr(p, LOWEST, TOKEN_EOF);
    loop->body = parse_stmt(p);

    return loop;
}

static ASTNode_T* parse_case(Parser_T* p)
{
    ASTNode_T* case_stmt = init_ast_node(ND_CASE, p->tok);

    if(tok_is(p, TOKEN_UNDERSCORE))
    {
        parser_advance(p);
        case_stmt->is_default_case = true;
    }
    else
        case_stmt->condition = parse_expr(p, LOWEST, TOKEN_ARROW);

    parser_consume(p, TOKEN_ARROW, "expect `=>` after case condition");
    case_stmt->body = parse_stmt(p);

    return case_stmt;
}

static ASTNode_T* parse_match(Parser_T* p)
{
    ASTNode_T* match = init_ast_node(ND_MATCH, p->tok);
    match->cases = init_list(sizeof(struct AST_NODE_STRUCT*));
    match->default_case = NULL;

    parser_consume(p, TOKEN_MATCH, "expect `match` keyword to match an expression");

    match->condition = parse_expr(p, LOWEST, TOKEN_LBRACE);
    
    parser_consume(p, TOKEN_LBRACE, "expect `{` after match condition");

    while(!tok_is(p, TOKEN_RBRACE) && !tok_is(p, TOKEN_EOF))
    {
        ASTNode_T* case_stmt = parse_case(p);

        if(case_stmt->is_default_case)
        {
            if(match->default_case)
                throw_error(ERR_REDEFINITION, p->tok, "redefinition of default case `_`.");

            match->default_case = case_stmt;
            continue;
        }

        list_push(match->cases, case_stmt);
    }

    parser_consume(p, TOKEN_RBRACE, "expect `}` after match condition");
    return match;
}

static ASTNode_T* parse_expr_stmt(Parser_T* p)
{
    ASTNode_T* stmt = init_ast_node(ND_EXPR_STMT, p->tok);
    stmt->expr = parse_expr(p, LOWEST, TOKEN_SEMICOLON);

    if(!is_executable(stmt->expr->kind))
        throw_error(ERR_SYNTAX_ERROR, stmt->expr->tok, "cannot treat `%s` as a statement, expect function call, assignment or similar", stmt->expr->tok->value);

    parser_consume(p, TOKEN_SEMICOLON, "expect `;` after expression statement");

    return stmt;
}

static void parse_local(Parser_T* p)
{
    ASTObj_T* local = init_ast_obj(OBJ_LOCAL, p->tok);
    parser_consume(p, TOKEN_LET, "expect `let` keyword for variable definition");
    
    local->callee = strdup(p->tok->value);
    parser_consume(p, TOKEN_ID, "expect variable name");
    parser_consume(p, TOKEN_COLON, "expect `:` after variable name");

    local->data_type = parse_type(p);

    if(tok_is(p, TOKEN_ASSIGN))
    {
        parser_advance(p);
        local->value = parse_expr(p, LOWEST, TOKEN_SEMICOLON);
    }
    
    parser_consume(p, TOKEN_SEMICOLON, "expect `;` after variable declaration");

    if(!p->current_block || p->current_block->kind != ND_BLOCK)
        throw_error(ERR_SYNTAX_ERROR, p->tok, "cannot define a local variable outside a block statement");
    list_push(p->current_block->locals, local);
}

static ASTNode_T* parse_stmt(Parser_T* p)
{
repeat:

    switch(p->tok->type)
    {
        case TOKEN_LBRACE:
            return parse_block(p);
        case TOKEN_RETURN:
            return parse_return(p);
        case TOKEN_IF:
            return parse_if(p);
        case TOKEN_LOOP:
            return parse_loop(p);
        case TOKEN_MATCH:
            return parse_match(p);
        case TOKEN_LET:
            parse_local(p);
            goto repeat;    // I know, gotos are bad, but in this case it's the most simple solution
        default:
            return parse_expr_stmt(p);
    }

    // satisfy -Wall
    return NULL;
}

/////////////////////////////////
// Expression PRATT parser     //
/////////////////////////////////

static ASTNode_T* parse_expr(Parser_T* p, Precedence_T prec, TokenType_T end_tok)
{
    prefix_parse_fn prefix = get_prefix_parse_fn(p->tok->type);

    if(!prefix)
        throw_error(ERR_SYNTAX_ERROR, p->tok, "unexpected token `%s`, expect expression", p->tok->value);

    ASTNode_T* left_expr = prefix(p);

    while(!tok_is(p, end_tok) && prec < get_precedence(p->tok->type))
    {
        infix_parse_fn infix = get_infix_parse_fn(p->tok->type);
        if(!infix)
            return left_expr;
        
        left_expr = infix(p, left_expr);
    }

    return left_expr;
}

static List_T* parse_expr_list(Parser_T* p, TokenType_T end_tok)
{
    List_T* list = init_list(sizeof(struct AST_NODE_STRUCT*));

    while (!tok_is(p, end_tok) && !tok_is(p, TOKEN_EOF)) 
    {
        list_push(list, parse_expr(p, LOWEST, TOKEN_COMMA));

        if(!tok_is(p, end_tok))
            parser_consume(p, TOKEN_COMMA, "expect `,` between call arguments");
    }

    return list;
}

static ASTNode_T* parse_id(Parser_T* p)
{
    ASTNode_T* id = init_ast_node(ND_ID, p->tok);
    id->callee = id->tok->value;
    parser_consume(p, TOKEN_ID, "expect an identifier");
    return id;
}

static ASTNode_T* parse_int_lit(Parser_T* p)
{
    ASTNode_T* int_lit = init_ast_node(ND_INT, p->tok);
    int_lit->int_val = atoi(p->tok->value);
    int_lit->is_constant = true;
    parser_consume(p, TOKEN_INT, "expect integer literal (0, 1, 2, ...)");
    return int_lit;
}

static ASTNode_T* parse_float_lit(Parser_T* p)
{
    ASTNode_T* float_lit = init_ast_node(ND_FLOAT, p->tok);
    float_lit->float_val = atof(p->tok->value);
    parser_consume(p, TOKEN_INT, "expect float literal (0, 1, 2.3, ...)");
    return float_lit;
}

static ASTNode_T* parse_bool_lit(Parser_T* p)
{
    ASTNode_T* bool_lit = constant_literals[p->tok->type];

    bool_lit->bool_val = p->tok->type == TOKEN_TRUE;

    parser_advance(p);

    if(!bool_lit->data_type)
        bool_lit->data_type = primitives[TY_BOOL];

    return bool_lit;
}

static ASTNode_T* parse_nil_lit(Parser_T* p)
{
    ASTNode_T* nil_lit = constant_literals[p->tok->type];
    parser_advance(p);

    if(!nil_lit->data_type)
        nil_lit->data_type = &(ASTType_T){.kind = TY_PTR, .is_primitive = true, .size = VOID_S, .base = primitives[TY_VOID]};

    return nil_lit;
}

static ASTNode_T* parse_char_lit(Parser_T* p)
{
    ASTNode_T* char_lit = init_ast_node(ND_CHAR, p->tok);
    char_lit->char_val = p->tok->value[0];
    char_lit->is_constant = true;
    parser_consume(p, TOKEN_CHAR, "expect char literal ('a', 'b', ...)");
    return char_lit;
}

static ASTNode_T* parse_str_lit(Parser_T* p)
{
    ASTNode_T* str_lit = init_ast_node(ND_STR, p->tok);
    str_lit->str_val = str_lit->tok->value;
    str_lit->is_constant = true;
    parser_consume(p, TOKEN_STRING, "expect string literal (\"abc\", \"wxyz\", ...)");
    return str_lit;
}

static ASTNode_T* parse_array_lit(Parser_T* p)
{
    ASTNode_T* arr_lit = init_ast_node(ND_ARRAY, p->tok);
    parser_consume(p, TOKEN_LBRACKET, "expect `[` for array literal");
    arr_lit->is_constant = true;
    arr_lit->args = parse_expr_list(p, TOKEN_RBRACKET);
    parser_consume(p, TOKEN_RBRACKET, "expect `]` after array literal");

    return arr_lit;
}

static ASTNode_T* parse_struct_lit(Parser_T* p)
{
    ASTNode_T* struct_lit = init_ast_node(ND_STRUCT, p->tok);
    parser_consume(p, TOKEN_LBRACE, "expect `{` for struct literal");
    struct_lit->is_constant = true;
    struct_lit->args = parse_expr_list(p, TOKEN_RBRACE);
    parser_consume(p, TOKEN_RBRACE, "expect `}` after struct literal");

    return struct_lit;
}

static ASTNode_T* parse_unary(Parser_T* p)
{
    ASTNode_T* unary = init_ast_node(unary_ops[p->tok->type], p->tok);
    parser_advance(p);

    unary->right = parse_expr(p, LOWEST, TOKEN_EOF);
    return unary;
}

static ASTNode_T* parse_num_op(Parser_T* p, ASTNode_T* left)
{
    ASTNode_T* infix = init_ast_node(infix_ops[p->tok->type], p->tok);
    parser_advance(p);

    infix->left = left;
    infix->right = parse_expr(p, expr_parse_fns[infix->tok->type].prec, TOKEN_EOF);

    return infix;
}

static ASTNode_T* parse_bool_op(Parser_T* p, ASTNode_T* left)
{
    ASTNode_T* infix = init_ast_node(infix_ops[p->tok->type], p->tok);
    parser_advance(p);

    infix->left = left;
    infix->right = parse_expr(p, expr_parse_fns[infix->tok->type].prec, TOKEN_EOF);

    infix->data_type = primitives[TY_BOOL]; // set the data type, since == != > >= < <= will always result in booleans

    return infix;
}

static ASTNode_T* generate_assignment_op_rval(Parser_T* p, ASTNode_T* left, TokenType_T op)
{
    ASTNode_T* rval = init_ast_node(infix_ops[op], p->tok);
    rval->left = left;
    rval->right = parse_expr(p, expr_parse_fns[op].prec, TOKEN_EOF);

    return rval;
}

static ASTNode_T* parse_assignment(Parser_T* p, ASTNode_T* left)
{
    if(!is_editable(left->kind))
        throw_error(ERR_SYNTAX_ERROR, p->tok, "cannot assign a value to `%s`, expect variable or similar", left->tok->value);

    ASTNode_T* assign = init_ast_node(ND_ASSIGN, p->tok);
    assign->left = left;

    switch(p->tok->type)
    {
        case TOKEN_ASSIGN:
            parser_advance(p);
            assign->right = parse_expr(p, expr_parse_fns[p->tok->type].prec, TOKEN_EOF);
            return assign;
        default:
            parser_advance(p);
            assign->right = generate_assignment_op_rval(p, left, p->tok->type);
            return assign;
    }

    // satisfy -Wall
    return NULL;
}

static ASTNode_T* parse_postfix(Parser_T* p, ASTNode_T* left)
{
    ASTNode_T* postfix = init_ast_node(infix_ops[p->tok->type], p->tok);
    postfix->left = left;

    parser_advance(p);

    return postfix;
}

static ASTNode_T* parse_call(Parser_T* p, ASTNode_T* left)
{
    if(!is_editable(left->kind))
        throw_error(ERR_SYNTAX_ERROR, p->tok, "cannot call `%s`, expect function name or similar", left->tok->value);

    ASTNode_T* call = init_ast_node(ND_CALL, p->tok);
    call->expr = left;  // the expression to call

    parser_consume(p, TOKEN_LPAREN, "expect `(` after callee");

    call->args = parse_expr_list(p, TOKEN_RPAREN);
    parser_consume(p, TOKEN_RPAREN, "expect `)` after call arguments");
    
    return call;
}

static ASTNode_T* parse_index(Parser_T* p, ASTNode_T* left)
{
    if(!is_editable(left->kind))
        throw_error(ERR_SYNTAX_ERROR, p->tok, "cannot get an index value of `%s`, expect array name or similar", left->tok->value);

    ASTNode_T* index = init_ast_node(ND_INDEX, p->tok);
    index->left = left;

    parser_consume(p, TOKEN_LBRACKET, "expect `[` after array name for an index expression");

    index->expr = parse_expr(p, LOWEST, TOKEN_RBRACKET);
    parser_consume(p, TOKEN_RBRACKET, "expect `]` after array index");

    return index;
}

static ASTNode_T* parse_member(Parser_T* p, ASTNode_T* left)
{
    if(!is_editable(left->kind))
        throw_error(ERR_SYNTAX_ERROR, p->tok, "cannot get a member of `%s`, expect struct name or similar", left->tok->value);

    ASTNode_T* member = init_ast_node(ND_MEMBER, p->tok);
    member->left = left;

    parser_consume(p, TOKEN_DOT, "expect `.` for a member expression");
    member->right = parse_expr(p, expr_parse_fns[TOKEN_DOT].prec, TOKEN_EOF);

    if(member->right->kind != ND_ID)
        throw_error(ERR_SYNTAX_ERROR, p->tok, "cannot get %s as a member of `%s`, expect member name", member->right->tok->value);

    return member;
}

static ASTNode_T* parse_closure(Parser_T* p)
{
    parser_consume(p, TOKEN_LPAREN, "expect `(` for closure");
    ASTNode_T* expr = parse_expr(p, LOWEST, TOKEN_RPAREN);
    parser_consume(p, TOKEN_RPAREN, "expect `)` after closure");

    return expr;
}