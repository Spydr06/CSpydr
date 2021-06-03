#include "parser.h"
#include "../io/log.h"
#include "../io/io.h"

#include "../ast/types.h"

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

static struct { prefix_parse_fn pfn; infix_parse_fn ifn; Precedence_T prec; } expr_parse_fns[TOKEN_EOF + 1] = {
    [TOKEN_ID]       = {parse_id, NULL, LOWEST},
    [TOKEN_INT]      = {parse_int_lit, NULL, LOWEST},
    [TOKEN_FLOAT]    = {parse_float_lit, NULL, LOWEST},
    [TOKEN_NIL]      = {parse_nil_lit, NULL, LOWEST},
    [TOKEN_TRUE]     = {parse_bool_lit, NULL, LOWEST},
    [TOKEN_FALSE]    = {parse_bool_lit, NULL, LOWEST},
    [TOKEN_CHAR]     = {parse_char_lit, NULL, LOWEST},
    [TOKEN_STRING]   = {parse_str_lit, NULL, LOWEST},
    [TOKEN_BANG]     = {NULL, NULL, LOWEST},
    [TOKEN_MINUS]    = {NULL, NULL, SUM},
    [TOKEN_LPAREN]   = {NULL, NULL, CALL}, 
    [TOKEN_LBRACKET] = {NULL, NULL, INDEX},   
    [TOKEN_LBRACE]   = {NULL, NULL, LOWEST}, 
    [TOKEN_STAR]     = {NULL, NULL, PRODUCT},
    [TOKEN_REF]      = {NULL, NULL, LOWEST},
    [TOKEN_TILDE]    = {NULL, NULL, LOWEST},
    [TOKEN_PLUS]     = {NULL, NULL, SUM},    
    [TOKEN_SLASH]    = {NULL, NULL, PRODUCT},    
    [TOKEN_EQ]       = {NULL, NULL, EQUALS}, 
    [TOKEN_NOT_EQ]   = {NULL, NULL, EQUALS},     
    [TOKEN_GT]       = {NULL, NULL, LTGT}, 
    [TOKEN_GT_EQ]    = {NULL, NULL, LTGT},    
    [TOKEN_LT]       = {NULL, NULL, LTGT}, 
    [TOKEN_LT_EQ]    = {NULL, NULL, LTGT},           
    [TOKEN_INC]      = {NULL, NULL, POSTFIX},  
    [TOKEN_DEC]      = {NULL, NULL, POSTFIX},  
    [TOKEN_ASSIGN]   = {NULL, NULL, ASSIGN},     
    [TOKEN_ADD]      = {NULL, NULL, ASSIGN},  
    [TOKEN_SUB]      = {NULL, NULL, ASSIGN},  
    [TOKEN_DIV]      = {NULL, NULL, ASSIGN},  
    [TOKEN_MULT]     = {NULL, NULL, ASSIGN},   
    [TOKEN_DOT]      = {NULL, NULL, MEMBER},  
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
    parser->eh = parser->lexer->eh;
    parser->tok = lexer_next_token(parser->lexer);
    parser->imports = init_list(sizeof(char*));

    parser->silent = false;

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
    {
        const char* err_tmp = "unexpected token `%s`, %s";
        char* err_msg = calloc(strlen(err_tmp) + strlen(p->tok->value) + strlen(msg) + 1, sizeof(char));
        sprintf(err_msg, err_tmp, p->tok->value, msg);
        throw_syntax_error(p->eh, err_msg, p->tok->line, p->tok->pos);
        free(err_msg);
        exit(1);
    }

    return parser_advance(p);
}

static ASTObj_T* parse_fn(Parser_T* p);

ASTProg_T* parse(Parser_T* p, const char* main_file)
{

    if(!p->silent)
    {
        LOG_OK_F(COLOR_BOLD_GREEN "  Compiling" COLOR_RESET " %s\n", main_file);
    }

    ASTProg_T* prog = init_ast_prog(main_file, NULL);

    while(!tok_is(p, TOKEN_EOF))
    {
        switch(p->tok->type)
        {
            case TOKEN_IMPORT:
                // TODO: handle imports
                break;
            case TOKEN_TYPE:
                // TODO: handle typedefs
                break;
            case TOKEN_LET:
                // TODO: handle globals
                break;
            case TOKEN_FN:
                list_push(prog->objs, parse_fn(p));
                break;
            default: {
                const char* err_tmp = "unexpected token `%s`, expect [import, type, let, fn]";
                char* err_msg = calloc(strlen(err_tmp) + strlen(p->tok->value) + 1, sizeof(char));
                sprintf(err_msg, err_tmp, p->tok->value);
                throw_syntax_error(p->eh, err_msg, p->tok->line, p->tok->pos);
            }
        }
    }

    return prog;
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
            // TODO: handle struct parsing
            break;
        case TOKEN_ENUM:
            // TODO: handle enum parsing
            break;
        case TOKEN_STAR:
            {
                type = init_ast_type(TY_PTR, p->tok);
                parser_advance(p);
                type->base = parse_type(p);
            }

            break;
        case TOKEN_LBRACKET:
            // TODO: handle array parsing
            break;
        default:
            // TODO: handle typedef parsing
            break;
    }

    return type;
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
static ASTNode_T* parse_expr(Parser_T* p, Precedence_T prec, TokenType_T end_tok);

static ASTObj_T* parse_fn(Parser_T* p)
{
    ASTObj_T* fn = init_ast_obj(OBJ_FUNCTION, p->tok);
    parser_consume(p, TOKEN_FN, "expect `fn` keyword for a function definition");

    fn->callee = strdup(p->tok->value);
    parser_consume(p, TOKEN_ID, "expect function name");

    parser_consume(p, TOKEN_LPAREN, "expect `(` after function name");

    fn->args = parse_argument_list(p, TOKEN_RPAREN);

    parser_consume(p, TOKEN_RPAREN, "expect `)` after function arguments");
    parser_consume(p, TOKEN_COLON, "expect `:` after function arguments");
    
    fn->return_type = parse_type(p);

    fn->body = parse_stmt(p);

    return fn;
}

static ASTNode_T* parse_block(Parser_T* p)
{
    ASTNode_T* block = init_ast_node(ND_BLOCK, p->tok);

    parser_consume(p, TOKEN_LBRACE, "expect `{` at the beginning of a block statement");

    while(p->tok->type != TOKEN_RBRACE)
        parse_stmt(p);

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
                throw_redef_error(p->eh, "redefinition of default case `_`.", case_stmt->tok->line, case_stmt->tok->pos + 1);

            match->default_case = case_stmt;
            continue;
        }

        list_push(match->cases, case_stmt);
    }

    parser_consume(p, TOKEN_RBRACE, "expect `}` after match condition");
    return match;
}

static ASTNode_T* parse_stmt(Parser_T* p)
{
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
        default: {
            const char* err_tmp = "unexpected token `%s`, expect statement";
            char* err_msg = calloc(strlen(err_tmp) + strlen(p->tok->value) + 1, sizeof(char));
            sprintf(err_msg, err_tmp, p->tok->value);
            throw_syntax_error(p->eh, err_msg, p->tok->line, p->tok->pos);
        }
    }

    // satisfy -Wall
    return NULL;
}

static ASTNode_T* parse_expr(Parser_T* p, Precedence_T prec, TokenType_T end_tok)
{
    prefix_parse_fn prefix = get_prefix_parse_fn(p->tok->type);
    if(!prefix)
    {
        const char* template = "no prefix parse function for token `%s` found";
        char* msg = calloc(strlen(template) + strlen(p->tok->value) + 1, sizeof(char));
        sprintf(msg, template, p->tok->value);

        throw_syntax_error(p->eh, msg, p->tok->line, p->tok->pos);
        free(msg);
        exit(1);
    }
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
    parser_consume(p, TOKEN_CHAR, "expect char literal ('a', 'b', ...)");
    return char_lit;
}

static ASTNode_T* parse_str_lit(Parser_T* p)
{
    ASTNode_T* str_lit = init_ast_node(ND_STR, p->tok);
    str_lit->str_val = str_lit->tok->value;
    parser_consume(p, TOKEN_STRING, "expect string literal (\"abc\", \"wxyz\", ...)");
    return str_lit;
}

/*

bool expr_is_executable(ASTExpr_T* expr)    // checks if the expression can be used as a statement ("executable" means it has to assign something)
{
    if(expr->type != EXPR_POSTFIX && expr->type != EXPR_INFIX && expr->type != EXPR_CALL)
        return false;
    else if(expr->type != EXPR_INFIX)
        return true;

    return ((ASTInfix_T*) expr->expr)->op == OP_ASSIGN;
}

/////////////////////////////////
// Parser                      //
/////////////////////////////////

static ASTFile_T* parse_file(Parser_T* parser, const char* file_path, ASTProgram_T* program_ref);

ASTProgram_T* parse(Parser_T* parser, const char* main_file)
{
    ASTProgram_T* program = init_ast_program(main_file);

    list_push(program->files, parse_file(parser, main_file, program));

    Preprocessor_T* pre = init_preprocessor(parser->eh);
    optimize_ast(pre, program);
    free_preprocessor(pre);

    return program;    
}

static ASTGlobal_T* parse_global(Parser_T* parser);
static ASTFunction_T* parse_function(Parser_T* parser);
static void parse_import(Parser_T* parser, ASTProgram_T* program_ref);
static ASTTypedef_T* parse_typedef(Parser_T* parser);
static ASTCompound_T* parse_compound(Parser_T* parser);

static ASTFile_T* parse_file(Parser_T* parser, const char* file_path, ASTProgram_T* program_ref)
{
    if(!parser->silent)
    {
        LOG_OK_F(COLOR_BOLD_GREEN "  Compiling" COLOR_RESET " \"%s\"\n", file_path);
    }
    ASTFile_T* root = init_ast_file(parser->lexer->file->path);

    while(!tok_is(parser, TOKEN_EOF))
    {
        switch(parser->tok->type)
        {
            case TOKEN_LET:
                list_push(root->globals, parse_global(parser));
                break;
            case TOKEN_FN:
                list_push(root->functions, parse_function(parser));
                break;
            case TOKEN_IMPORT:
                parse_import(parser, program_ref);
                break;
            case TOKEN_TYPE:
                list_push(root->types, parse_typedef(parser));
                break;

            default:
                throw_syntax_error(parser->eh, "unexpected token", parser->tok->line, parser->tok->pos);
                break;
        }
    }

    return root;
}

static ASTStructType_T* parse_struct_type(Parser_T* parser);
static ASTEnumType_T* parse_enum_type(Parser_T* parser);

static ASTType_T* parse_type(Parser_T* parser)
{
    char* type = strdup(parser->tok->value);

    ASTType_T* primitive = get_primitive_type(type);
    if(primitive)
    {
        parser_advance(parser);
        primitive->line = parser->tok->line; //FIXME:
        primitive->pos = parser->tok->pos;   //FIXME:

        free(type);

        return primitive;
    }

    ASTDataType_T dt = AST_TYPEDEF;
    void* body = NULL;
    ASTType_T* subtype = NULL;

    if(streq(type, "struct")) 
    {
        dt = AST_STRUCT;
        body = parse_struct_type(parser);
    }
    else if(streq(type, "enum"))
    {
        dt = AST_ENUM;
        body = parse_enum_type(parser);
    }
    else if(streq(type, "*"))
    {
        parser_consume(parser, TOKEN_STAR, "expect `*` for pointer type");
        dt = AST_POINTER;
        subtype = parse_type(parser);
    }
    else if(streq(type, "["))
    {
        parser_consume(parser, TOKEN_LBRACKET, "expect `[` for array type");
        parser_consume(parser, TOKEN_RBRACKET, "expect `]` for array type");
        dt = AST_ARRAY;
        subtype = parse_type(parser);
    }

    if(dt == AST_TYPEDEF)   // no special type was found, skip.
        parser_advance(parser);

    ASTType_T* t = init_ast_type(dt, subtype, body, type, parser->tok->line, parser->tok->pos);
    free(type);
    return t;
}

static ASTStructType_T* parse_struct_type(Parser_T* parser)
{
    parser_consume(parser, TOKEN_STRUCT, "expect `struct` keyword");
    parser_consume(parser, TOKEN_LBRACE, "expect `{` after struct");

    List_T* names = init_list(sizeof(char*));
    List_T* types = init_list(sizeof(ASTType_T*));

    while(!tok_is(parser, TOKEN_RBRACE))
    {
        list_push(names, strdup(parser->tok->value));
        parser_consume(parser, TOKEN_ID, "expect struct field name");
        parser_consume(parser, TOKEN_COLON, "expect `:` after field name");

        list_push(types, parse_type(parser));
        
        if(tok_is(parser, TOKEN_EOF))
        {
            throw_syntax_error(parser->eh, "unclosed struct body", parser->tok->line, parser->tok->pos);
            exit(1);
        }
        else if(!tok_is(parser, TOKEN_RBRACE))
            parser_consume(parser, TOKEN_COMMA, "expect `,` between struct fields");
    }
    parser_advance(parser);

    if(names->size != types->size)
        LOG_ERROR_F("struct fields have different size: {names: %ld, types: %ld}", names->size, types->size);

    return init_ast_struct_type(types, names);
}

static ASTEnumType_T* parse_enum_type(Parser_T* parser)
{
    parser_consume(parser, TOKEN_ENUM, "expect `enum` keyword");
    parser_consume(parser, TOKEN_LBRACE, "expect `{` after enum");

    List_T* fields = init_list(sizeof(char*));
    
    while(!tok_is(parser, TOKEN_RBRACE))
    {
        list_push(fields, strdup(parser->tok->value));
        parser_consume(parser, TOKEN_ID, "expect enum field name");

        if(tok_is(parser, TOKEN_EOF))
        {
            throw_syntax_error(parser->eh, "unclosed enum body", parser->tok->line, parser->tok->pos);
            exit(1);
        }
        else if(!tok_is(parser, TOKEN_RBRACE))
            parser_consume(parser, TOKEN_COMMA, "expect `,` between enum fields");
    }
    parser_advance(parser);
    return init_ast_enum_type(fields);
}

static ASTExpr_T* parse_expr(Parser_T* parser, precedence_T precedence);

static List_T* parse_expression_list(Parser_T* parser, TokenType_T end)
{
    List_T* exprs = init_list(sizeof(struct AST_EXPRESSION_STRUCT*));

    unsigned int start_line = parser->tok->line, start_pos = parser->tok->pos;

    while(!tok_is(parser, end))
    {
        list_push(exprs, parse_expr(parser, LOWEST));
        
        if(!tok_is(parser, end))
            parser_consume(parser, TOKEN_COMMA, "expect `,` between expressions");
        else if(tok_is(parser, TOKEN_EOF))
        {
            throw_syntax_error(parser->eh, "unclosed expession list", start_line, start_pos);
            exit(1);
        }
    }

    return exprs;
}

/////////////////////////////////
// Expressin PRATT parser      //
/////////////////////////////////

static ASTExpr_T* parse_expr(Parser_T* parser, precedence_T precedence)
{
    prefix_parse_fn prefix = get_prefix_parse_fn(parser->tok->type);
    if(prefix == NULL)
    {
        const char* template = "no prefix parse functio for `%s` found";
        char* msg = calloc(strlen(template) + strlen(parser->tok->value) + 1, sizeof(char));
        sprintf(msg, template, parser->tok->value);

        throw_syntax_error(parser->eh, msg, parser->tok->line, parser->tok->pos);
        free(msg);
        exit(1);
    }
    ASTExpr_T* left_expr = prefix(parser);

    while(!tok_is(parser, TOKEN_SEMICOLON) && precedence < get_precedence(parser->tok))
    {
        infix_parse_fn infix = get_Infix_parse_fn(parser->tok->type);
        if(infix == NULL)
            return left_expr;
        
        left_expr = infix(parser, left_expr);
    }

    return left_expr;
}

static ASTExpr_T* parse_identifier(Parser_T* parser) 
{
    ASTIdentifier_T* id = init_ast_identifier(parser->tok->value, NULL);
    parser_consume(parser, TOKEN_ID, "expect identifier");

    if(tok_is(parser, TOKEN_DOT))
    {
        parser_advance(parser);
        ASTExpr_T* expr = parse_identifier(parser);
        id->child_id = ((ASTIdentifier_T*) expr->expr);
        free(expr);
    }

    return init_ast_expr(NULL, EXPR_IDENTIFIER, id);
}

static ASTExpr_T* parse_int(Parser_T* parser)
{
    ASTExpr_T* ast = init_ast_expr(primitives[AST_I32], EXPR_INT_LITERAL, init_ast_int(atoi(parser->tok->value)));
    parser_consume(parser, TOKEN_INT, "expect number");
    return ast;
}

static ASTExpr_T* parse_float(Parser_T* parser)
{
    ASTExpr_T* ast = init_ast_expr(primitives[AST_F32], EXPR_FLOAT_LITERAL, init_ast_float(atof(parser->tok->value)));
    parser_consume(parser, TOKEN_FLOAT, "expect number");
    return ast;
}

static ASTExpr_T* parse_bool(Parser_T* parser)
{
    bool boolVal;
    if(tok_is(parser, TOKEN_TRUE))
        boolVal = true;
    else if(tok_is(parser, TOKEN_FALSE))
        boolVal = false;
    else {
        throw_syntax_error(parser->eh, "not a bool value", parser->tok->line, parser->tok->pos);
        exit(1);
    }

    ASTExpr_T* ast = init_ast_expr(primitives[AST_BOOL], EXPR_BOOL_LITERAL, init_ast_bool(boolVal));
    parser_advance(parser);
    return ast;
}

static ASTExpr_T* parse_char(Parser_T* parser)
{
    ASTExpr_T* ast = init_ast_expr(primitives[AST_CHAR], EXPR_CHAR_LITERAL, init_ast_char(parser->tok->value[0]));
    parser_consume(parser, TOKEN_CHAR, "expect character");
    return ast;
}

static ASTExpr_T* parse_string(Parser_T* parser)
{
    ASTExpr_T* ast = init_ast_expr(init_ast_type(AST_POINTER, primitives[AST_CHAR], NULL, "", parser->tok->line, parser->tok->pos), EXPR_STRING_LITERAL, init_ast_string(parser->tok->value));
    parser_consume(parser, TOKEN_STRING, "expect string");
    return ast;
}

static ASTExpr_T* parse_nil(Parser_T* parser)
{
    parser_consume(parser, TOKEN_NIL, "expect `nil`");
    return init_ast_expr(init_ast_type(AST_POINTER, primitives[AST_VOID], NULL, "", parser->tok->line, parser->tok->pos), EXPR_NIL, init_ast_nil());  // nil is just *void 0
}

static ASTExpr_T* parse_array(Parser_T* parser)
{
    parser_consume(parser, TOKEN_LBRACKET, "expect `[` for array literal");
    List_T* indexes = parse_expression_list(parser, TOKEN_RBRACKET);
    parser_advance(parser);

    return init_ast_expr(init_ast_type(AST_ARRAY, NULL, NULL, "", parser->tok->line, parser->tok->pos), EXPR_ARRAY_LITERAL, init_ast_array(indexes));
}

static ASTExpr_T* parse_struct(Parser_T* parser)
{
    parser_consume(parser, TOKEN_LBRACE, "expect `{` for struct literal");
    List_T* fields = init_list(sizeof(char*));
    List_T* exprs = init_list(sizeof(struct AST_EXPRESSION_STRUCT*));

    unsigned int start_line = parser->tok->line, start_pos = parser->tok->pos;

    while(!tok_is(parser, TOKEN_RBRACE))
    {
        list_push(fields, strdup(parser->tok->value));
        parser_consume(parser, TOKEN_ID, "expect struct field name");
        parser_consume(parser, TOKEN_COLON, "expect `:` after struct field");
        list_push(exprs, parse_expr(parser, LOWEST));

        if(tok_is(parser, TOKEN_EOF))
        {
            throw_syntax_error(parser->eh, "unclosed struct literal body, expect `}`", start_line, start_pos);
            exit(1);
        }
        else if(!tok_is(parser, TOKEN_RBRACE))
            parser_consume(parser, TOKEN_COMMA, "expect `,` between struct fields");
    }
    parser_advance(parser);

    return init_ast_expr(init_ast_type(AST_STRUCT, NULL, init_ast_struct_type(init_list(sizeof(struct AST_TYPE_STRUCT*)), init_list(sizeof(char*))), "", parser->tok->line, parser->tok->pos), EXPR_STRUCT_LITERAL, init_ast_struct(exprs, fields));
}

static ASTExpr_T* parse_infix_expression(Parser_T* parser, ASTExpr_T* left)
{
    precedence_T prec = get_precedence(parser->tok);
    ASTInfixOpType_T op;

    switch(parser->tok->type)
    {
        case TOKEN_PLUS:
            op = OP_ADD;
            break;
        case TOKEN_MINUS:
            op = OP_SUB;
            break;
        case TOKEN_STAR:
            op = OP_MULT;
            break;
        case TOKEN_SLASH:
            op = OP_DIV;
            break;
        case TOKEN_EQ:
            op = OP_EQ;
            break;
        case TOKEN_NOT_EQ:
            op = OP_NOT_EQ;
            break;
        case TOKEN_GT:
            op = OP_GT;
            break;
        case TOKEN_LT:
            op = OP_LT;
            break;
        case TOKEN_LT_EQ:
            op = OP_LT_EQ;
            break;
        case TOKEN_GT_EQ:
            op = OP_LT_EQ;
            break;
        default:
            throw_syntax_error(parser->eh, "undefined infix expression", parser->tok->line, parser->tok->pos);
            exit(1);
            break;
    }
    parser_advance(parser);
    ASTExpr_T* right = parse_expr(parser, prec);

    return init_ast_expr(NULL, EXPR_INFIX, init_ast_infix(op, right, left));
}

static ASTExpr_T* parse_call_expression(Parser_T* parser, ASTExpr_T* left)
{
    if(left->type != EXPR_IDENTIFIER)
    {
        throw_syntax_error(parser->eh, "expect method name for call", parser->tok->line, parser->tok->pos);
        exit(1);
    }

    parser_consume(parser, TOKEN_LPAREN, "epxect `(` for function call");
    List_T* args = parse_expression_list(parser, TOKEN_RPAREN);
    parser_consume(parser, TOKEN_RPAREN, "expect `)` after function call arguments");    

    ASTExpr_T* ast = init_ast_expr(NULL, EXPR_CALL, init_ast_call(((ASTIdentifier_T*) left->expr)->callee, args));
    free_ast_expr(left);  // free the left ast node because we only store the callee
    return ast;
}

static ASTExpr_T* parse_index_expression(Parser_T* parser, ASTExpr_T* left)
{
    parser_consume(parser, TOKEN_LBRACKET, "epxect `[` for index expression");
    ASTExpr_T* index = parse_expr(parser, LOWEST);
    parser_consume(parser, TOKEN_RBRACKET, "expect `]` after array index");

    return init_ast_expr(NULL, EXPR_INDEX, init_ast_index(left, index));
}

static ASTExpr_T* parse_not(Parser_T* parser)
{
    parser_consume(parser, TOKEN_BANG, "expect `!` for `not` operator");
    return init_ast_expr(init_ast_type(AST_BOOL, NULL, NULL, "", parser->tok->line, parser->tok->pos), EXPR_PREFIX, init_ast_prefix(OP_NOT, parse_expr(parser, LOWEST)));
}

static ASTExpr_T* parse_negate(Parser_T* parser)
{
    parser_consume(parser, TOKEN_MINUS, "expect `-` for `negate` operator");
    return init_ast_expr(NULL, EXPR_PREFIX, init_ast_prefix(OP_NEGATE, parse_expr(parser, LOWEST)));
}

static ASTExpr_T* parse_deref(Parser_T* parser)
{
    parser_consume(parser, TOKEN_STAR, "expect `*` to dereference a pointer");
    return init_ast_expr(NULL, EXPR_PREFIX, init_ast_prefix(OP_DEREF, parse_expr(parser, LOWEST)));
}

static ASTExpr_T* parse_ref(Parser_T* parser)
{
    parser_consume(parser, TOKEN_REF, "expect `&` to get a pointer");
    return init_ast_expr(NULL, EXPR_PREFIX, init_ast_prefix(OP_REF, parse_expr(parser, LOWEST)));
}

static ASTExpr_T* parse_bitwise_negation(Parser_T* parser)
{
    parser_consume(parser, TOKEN_TILDE, "expect `~` for bitwise negation");
    return init_ast_expr(NULL, EXPR_PREFIX, init_ast_prefix(OP_BIT_NEG, parse_expr(parser, LOWEST)));
}

static ASTExpr_T* parse_closure(Parser_T* parser)
{
    parser_consume(parser, TOKEN_LPAREN, "expect `(` for closure");
    ASTExpr_T* ast = parse_expr(parser, LOWEST);
    parser_consume(parser, TOKEN_RPAREN, "expect `)` after closure");
    return ast;
}

static ASTExpr_T* parse_postfix_expression(Parser_T* parser, ASTExpr_T* left)
{
    ASTPostfixOpType_T op;

    switch(parser->tok->type)
    {
        case TOKEN_INC:
            op = OP_INC;
            break;

        case TOKEN_DEC:
            op = OP_DEC;
            break;

        default:
            throw_syntax_error(parser->eh, "expect `++` or `--`", parser->tok->line, parser->tok->pos);
            exit(1);
    }
    parser_advance(parser);

    return init_ast_expr(NULL, EXPR_POSTFIX, init_ast_postfix(op, left));
}

static ASTExpr_T* parse_assignment_op(ASTExpr_T* left, ASTInfixOpType_T op, ASTExpr_T* right)
{
    return init_ast_expr(NULL, EXPR_INFIX, 
                init_ast_infix(OP_ASSIGN, init_ast_expr(
                    NULL, EXPR_INFIX, 
                    init_ast_infix(op, right, left)), 
                    init_ast_expr(
                        NULL, EXPR_IDENTIFIER, 
                        init_ast_identifier(((ASTIdentifier_T*) left->expr)->callee, NULL)
                    )
                )
            );
}

static ASTExpr_T* parse_assignment(Parser_T* parser, ASTExpr_T* left)
{
    if(left->type != EXPR_IDENTIFIER && left->type != EXPR_INDEX)
    {
        throw_syntax_error(parser->eh, "can only assing a value to a variable", parser->tok->line, parser->tok->pos);
        exit(1);
    }

    TokenType_T op = parser->tok->type;
    parser_advance(parser);
    
    ASTExpr_T* right = parse_expr(parser, ASSIGN);
    switch(op)
    {
        case TOKEN_ASSIGN:
            return init_ast_expr(NULL, EXPR_INFIX, init_ast_infix(OP_ASSIGN, right, left));
        case TOKEN_ADD:
            return parse_assignment_op(left, OP_ADD, right);
        case TOKEN_SUB:
            return parse_assignment_op(left, OP_SUB, right);
        case TOKEN_MULT:
            return parse_assignment_op(left, OP_MULT, right);
        case TOKEN_DIV:
            return parse_assignment_op(left, OP_DIV, right);
        default:
            throw_syntax_error(parser->eh, "unexpected token, expect assignment", parser->tok->line, parser->tok->pos);
            exit(1);
    }
}

/////////////////////////////////
// Statements                  //
/////////////////////////////////

static ASTLoop_T* parse_loop(Parser_T* parser) // TODO: loops will for now only support while-like syntax; for and foreach come soon
{
    parser_consume(parser, TOKEN_LOOP, "expect `loop` keyword");

    ASTExpr_T* condition = parse_expr(parser, LOWEST);
    ASTCompound_T* body = parse_compound(parser);

    return init_ast_loop(condition, body);
}

static ASTMatch_T* parse_match(Parser_T* parser)
{
    parser_consume(parser, TOKEN_MATCH, "expect `match` keyword");
    
    ASTExpr_T* condition = parse_expr(parser, LOWEST);
    parser_consume(parser, TOKEN_LBRACE, "expect `{` after match condtion");

    List_T* cases = init_list(sizeof(struct AST_EXPRESSION_STRUCT*));
    List_T* bodys = init_list(sizeof(struct AST_COMPOUND_STRUCT*));
    ASTCompound_T* default_case = NULL;
    while(!tok_is(parser, TOKEN_RBRACE))
    {   
        switch(parser->tok->type)
        {
            case TOKEN_UNDERSCORE:
                if(default_case == NULL) {
                    parser_consume(parser, TOKEN_UNDERSCORE, "expect `_` for default case");
                    parser_consume(parser, TOKEN_ARROW, "expect `=>` after match case");
                    default_case = parse_compound(parser);
                } else
                {
                    throw_redef_error(parser->eh, "redefinition of default match case", parser->tok->line, parser->tok->pos);
                    exit(1);
                }
                break;

            case TOKEN_EOF:
                throw_syntax_error(parser->eh, "expect '}' after match statement", parser->tok->line, parser->tok->pos);
                exit(1);
                break;

            default:
                list_push(cases, parse_expr(parser, LOWEST));
                parser_consume(parser, TOKEN_ARROW, "expect `=>` after match case");
                list_push(bodys, parse_compound(parser));
                break;
        }
    }
    parser_advance(parser);

    return init_ast_match(condition, cases, bodys, default_case);
}

static ASTIf_T* parse_if(Parser_T* parser)
{
    parser_consume(parser, TOKEN_IF, "expect `if` keyword");
    ASTExpr_T* condition = parse_expr(parser, LOWEST);
    ASTCompound_T* if_body = parse_compound(parser);
    ASTCompound_T* else_body = NULL;

    if(tok_is(parser, TOKEN_ELSE)) {
        parser_advance(parser);
        else_body = parse_compound(parser);
    }
    
    return init_ast_if(condition, if_body, else_body);
}

static ASTReturn_T* parse_return(Parser_T* parser)
{
    parser_consume(parser, TOKEN_RETURN, "expect `ret` keyword");
    ASTReturn_T* ast = init_ast_return(parse_expr(parser, LOWEST));
    parser_consume(parser, TOKEN_SEMICOLON, "expect `;` after return value");
    return ast;
}

static ASTLocal_T* parse_local(Parser_T* parser)
{
    parser_consume(parser, TOKEN_LET, "expect `let` keyword");
    bool mutable = false;
    if(tok_is(parser, TOKEN_MUT))
    {
        mutable = true;
        parser_advance(parser);
    }

    char* name = strdup(parser->tok->value);
    parser_consume(parser, TOKEN_ID, "expect variable name");
    parser_consume(parser, TOKEN_COLON, "expect `:` after variable name");

    ASTType_T* type = parse_type(parser);

    ASTExpr_T* value = NULL;
    if(tok_is(parser, TOKEN_ASSIGN)) {
        parser_advance(parser); 
        value = parse_expr(parser, LOWEST);
    }
    
    parser_consume(parser, TOKEN_SEMICOLON, "expect `;` after variable definition");

    ASTLocal_T* ast = init_ast_local(type, value, name, parser->tok->line, parser->tok->pos);
    ast->isMutable = mutable;
    free(name);
    return ast;
}

static ASTExprStmt_T* parse_expression_statement(Parser_T* parser)
{
    unsigned int start_line = parser->tok->line, start_pos = parser->tok->pos + 1;

    ASTExprStmt_T* ast = init_ast_expr_stmt(parse_expr(parser, LOWEST));

    if(!expr_is_executable(ast->expr))
        throw_syntax_error(parser->eh, "can only treat assigning expressions as statements (e.g. =, +=, ++)", start_line, start_pos);

    parser_consume(parser, TOKEN_SEMICOLON, "expect `;` after expression");
    return ast;
}

static ASTStmt_T* parse_statement(Parser_T* parser)
{
    void* stmt;
    ASTStmtType_T type;

    switch(parser->tok->type)
    {
        case TOKEN_LOOP:
            stmt = parse_loop(parser);
            type = STMT_LOOP;
            break;
        
        case TOKEN_MATCH:
            stmt = parse_match(parser);
            type = STMT_MATCH;
            break;

        case TOKEN_IF:
            stmt = parse_if(parser);
            type = STMT_IF;
            break;

        case TOKEN_RETURN:
            type = STMT_RETURN;
            stmt = parse_return(parser);
            break;

        case TOKEN_LET:
            type = STMT_LET;
            stmt = parse_local(parser);
            break;

        case TOKEN_ID:
            type = STMT_EXPRESSION;
            stmt = parse_expression_statement(parser);
            break;
        
        default:
            throw_syntax_error(parser->eh, "expect statement", parser->tok->line, parser->tok->pos);
            exit(1);
            break;
    }

    return init_ast_stmt(type, stmt);
}

/////////////////////////////////
// base structures             //
/////////////////////////////////

static ASTCompound_T* parse_compound(Parser_T* parser)
{
    List_T* stmts = init_list(sizeof(struct AST_STATEMENT_STRUCT*));

    if(tok_is(parser, TOKEN_LBRACE))
    {
        parser_advance(parser);
        
        while(!tok_is(parser, TOKEN_RBRACE))
        {
            list_push(stmts, parse_statement(parser));

            if(tok_is(parser, TOKEN_EOF))
            {
                throw_syntax_error(parser->eh, "expect '}' after compound", parser->tok->line, parser->tok->pos);
                exit(1);
            }
        }
        parser_advance(parser);
    } else  // enables to do single statement compounds without braces
    {
        list_push(stmts, parse_statement(parser));
    }

    return init_ast_compound(stmts);
}

static ASTGlobal_T* parse_global(Parser_T* parser)
{
    parser_consume(parser, TOKEN_LET, "expect `let` keyword");

    bool mutable = false;
    if(tok_is(parser, TOKEN_MUT))
    {
        parser_advance(parser);
        mutable = true;
    }

    char* name = strdup(parser->tok->value);

    unsigned int line = parser->tok->line;
    unsigned int pos = parser->tok->pos;

    parser_consume(parser, TOKEN_ID, "expect variable name");
    parser_consume(parser, TOKEN_COLON, "expect `:` after variable name");

    ASTType_T* type = parse_type(parser);

    ASTExpr_T* value = NULL;
    if(tok_is(parser, TOKEN_ASSIGN)) {
        parser_advance(parser); 
        value = parse_expr(parser, LOWEST);
    }
    
    parser_consume(parser, TOKEN_SEMICOLON, "expect `;` after variable definition");

    ASTGlobal_T* ast = init_ast_global(name, type, value, line, pos);
    ast->is_mutable = mutable;
    free(name);
    return ast;
}

static ASTFunction_T* parse_function(Parser_T* parser)
{
    parser_consume(parser, TOKEN_FN, "expect `fn` keyword");
    char* name = strdup(parser->tok->value);

    unsigned int line = parser->tok->line;
    unsigned int pos = parser->tok->pos;

    parser_consume(parser, TOKEN_ID, "expect function name");
    parser_consume(parser, TOKEN_LPAREN, "expect `(` after function name");

    List_T* args = init_list(sizeof(struct AST_ARGUMENT_STRUCT*));
    while(!tok_is(parser, TOKEN_RPAREN))
    {
        char* argName = strdup(parser->tok->value);
        parser_consume(parser, TOKEN_ID, "expect argument name");
        parser_consume(parser, TOKEN_COLON, "expect `:` after argument name");
        list_push(args, init_ast_argument(argName, parse_type(parser)));
        free(argName);

        if(!tok_is(parser, TOKEN_RPAREN))
            parser_consume(parser, TOKEN_COMMA, "expect `,` between arguments");
    }
    parser_advance(parser);
    
    ASTType_T* returnType = NULL;
    if(tok_is(parser, TOKEN_COLON)) {
        parser_advance(parser);
        returnType = parse_type(parser);
    }
    else {
        returnType = init_ast_type(AST_VOID, NULL, NULL, "", parser->tok->line, parser->tok->pos);
    }

    ASTCompound_T* body = parse_compound(parser);

    ASTFunction_T* ast = init_ast_function(name, returnType, body, args, line, pos);
    free(name);
    return ast;
}

static char* getDirectoryFromRelativePath(char* mainPath)
{
#ifdef __linux__
    char* fullPath = realpath(mainPath, NULL);
    if(fullPath == NULL)
        return NULL;

    return dirname(fullPath);
#endif
}

static void parse_import(Parser_T* parser, ASTProgram_T* program_ref)
{
    parser_consume(parser, TOKEN_IMPORT, "expect `import` keyword");
    char* relativePath = strdup(parser->tok->value);
    parser_consume(parser, TOKEN_STRING, "expect filepath to import");

    char* directory = getDirectoryFromRelativePath(program_ref->main_file);
    char* importPath = calloc(strlen(directory) + strlen(relativePath) + 2, sizeof(char*));
    sprintf(importPath, "%s/%s", directory, relativePath);
    free(relativePath);
    free(directory);

    if(access(importPath, F_OK) != 0)
    {
        const char* template = "could not open file \"%s\": no such file or directory";
        char* message = calloc(strlen(template) + strlen(importPath) + 1, sizeof(char));
        sprintf(message, template, importPath);
        throw_undef_error(parser->eh, message, parser->tok->line, parser->tok->pos);
        free(message);
        exit(1);
    }

    parser_consume(parser, TOKEN_SEMICOLON, "expect `;` after import");

    for(int i = 0; i < parser->imports->size; i++)  // check if the file is already included, when its included, skip it. 
                                                    // No error gets thrown, because 2 files including each other is valid in Spydr
    {
        if(strcmp(parser->imports->items[i], importPath) == 0) {
            free(importPath);
            return;
        }
    }
    list_push(parser->imports, importPath);

    // if the file is not included, compile it to a new ASTFile_T.
    SrcFile_T* file = read_file(importPath);
    ErrorHandler_T* eh = init_errorhandler(file);
    Lexer_T* lexer = init_lexer(file, eh);
    Parser_T* _parser = init_parser(lexer);

    ASTFile_T* ast = parse_file(_parser, importPath, program_ref);
    list_push(program_ref->files, ast);

    free_parser(_parser);
    free_lexer(lexer);
    free_errorhandler(eh);
    free_srcfile(file);
}

static ASTTypedef_T* parse_typedef(Parser_T* parser)
{
    parser_consume(parser, TOKEN_TYPE, "expect `type` keyword");
    char* name = strdup(parser->tok->value);

    unsigned int line = parser->tok->line;
    unsigned int pos = parser->tok->pos;

    parser_consume(parser, TOKEN_ID, "expect type name");

    parser_consume(parser, TOKEN_COLON, "expect `:` after typename");

    ASTTypedef_T* ast = init_ast_typedef(parse_type(parser), name, line, pos);
    free(name);

    parser_consume(parser, TOKEN_SEMICOLON, "expect `;` after type defintion");
    return ast;
}*/