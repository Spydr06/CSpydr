#include "parser.h"

#include "ast/ast.h"
#include "hashmap.h"
#include "io/file.h"
#include "util.h"
#include "config.h"
#include "error/error.h"
#include "lexer/token.h"
#include "list.h"
#include "validator.h"
#include "io/log.h"
#include "io/io.h"
#include "ast/types.h"
#include "mem/mem.h"
#include "platform/platform_bindings.h"
#include "lexer/lexer.h"
#include "preprocessor/preprocessor.h"
#include "toolchain.h"
#include "codegen/codegen_utils.h"
#include "utils.h"
#include "optimizer/constexpr.h"
#include "timer/timer.h"
#include "codegen/transpiler/c_codegen.h"
#include "platform/pkg_config.h"
#include "directives.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include<unistd.h> 
#include <errno.h>

#define STATIC_MEMBER "::"

typedef struct PARSER_STRUCT
{
    Context_T* context;
    List_T* tokens;
    size_t token_i;
    ASTProg_T* root_ref;
    Token_T* tok;
    ASTNode_T* cur_block;
    ASTObj_T* cur_obj;

    size_t cur_lambda_id;
    size_t cur_tuple_id;

    const HashMap_T* operator_context;

    bool holes_enabled;
} Parser_T;

typedef enum 
{
    PREC_LOWEST = 0,

    PREC_ASSIGN,            // x = y
    PREC_PIPE,              // x |> y
    PREC_CUSTOM_OPERATOR,   // custom operators
    PREC_LOGIC_OR,          // x || y
    PREC_LOGIC_AND,         // x && y
    PREC_INFIX_CALL,        // x `y` z
    PREC_CAST,              // x: y
    PREC_BIT_OR,            // x | y
    PREC_BIT_XOR,           // x ^ y
    PREC_BIT_AND,           // x & y
    PREC_EQUALS,            // x == y
    PREC_LT,                // x < y
    PREC_GT = PREC_LT,      // x > y
    PREC_BIT_SHIFT,         // x << y
    PREC_PLUS,              // x + y
    PREC_MINUS = PREC_PLUS, // x - y
    PREC_MULT,              // x * y
    PREC_DIV = PREC_MULT,   // x / y
    PREC_MOD,               // x % y
    PREC_UNARY,             // -x, ~x, &x, *x
    PREC_POWER,             // x²
    PREC_INC,               // x--
    PREC_DEC = PREC_INC,    // x++
    PREC_X_OF,              // alignof x, sizeof x, typeof x
    PREC_CALL,              // x(y)
    PREC_ARRAY,             // x[y]
    PREC_MEMBER,            // x.y

    PREC_HIGHEST
} Precedence_T;

/////////////////////////////////
// expression parsing settings //
/////////////////////////////////

typedef ASTNode_T* (*PrefixParseFn_T)(Parser_T* parser);
typedef ASTNode_T* (*InfixParseFn_T)(Parser_T* parser, ASTNode_T* left);

static ASTNode_T* parse_id(Parser_T* p);
static ASTNode_T* parse_int_lit(Parser_T* p);
static ASTNode_T* parse_float_lit(Parser_T* p);
static ASTNode_T* parse_char_lit(Parser_T* p);
static ASTNode_T* parse_bool_lit(Parser_T* p);
static ASTNode_T* parse_nil_lit(Parser_T* p);
static ASTNode_T* parse_closure(Parser_T* p);

static ASTNode_T* parse_str_lit(Parser_T* p);

static ASTNode_T* parse_array_lit(Parser_T* p);
static ASTNode_T* parse_struct_lit(Parser_T* p, ASTNode_T* id);
static ASTNode_T* parse_anonymous_struct_lit(Parser_T* p);

static ASTNode_T* parse_lambda_lit(Parser_T* p);
static ASTNode_T* parse_ternary(Parser_T* p);
static ASTNode_T* parse_else_expr(Parser_T* p, ASTNode_T* left);

static ASTNode_T* parse_sizeof(Parser_T* p);
static ASTNode_T* parse_alignof(Parser_T* p);
static ASTNode_T* parse_len(Parser_T* p);

static ASTNode_T* parse_unary(Parser_T* p);
static ASTNode_T* parse_num_op(Parser_T* p, ASTNode_T* left);
static ASTNode_T* parse_bit_op(Parser_T* p, ASTNode_T* left);
static ASTNode_T* parse_bool_op(Parser_T* p, ASTNode_T* left);
static ASTNode_T* parse_assignment(Parser_T* p, ASTNode_T* left);
static ASTNode_T* parse_postfix(Parser_T* p, ASTNode_T* left);
static ASTNode_T* parse_index(Parser_T* p, ASTNode_T* left);

static ASTNode_T* parse_type_expr(Parser_T* p);

static ASTNode_T* parse_infix_call(Parser_T* p, ASTNode_T* left);

static ASTNode_T* parse_call(Parser_T* p, ASTNode_T* left);
static ASTNode_T* parse_custom_infix_operator(Parser_T* p, ASTNode_T* left);
static ASTNode_T* parse_custom_prefix_operator(Parser_T* p);
static ASTNode_T* parse_cast(Parser_T* p, ASTNode_T* left);
static ASTNode_T* parse_member(Parser_T* p, ASTNode_T* left);
static ASTNode_T* parse_pipe(Parser_T* p, ASTNode_T* left);
static ASTNode_T* parse_hole(Parser_T* p);
static ASTNode_T* parse_const_expr(Parser_T* p);

static ASTNode_T* parse_pow_2(Parser_T* p, ASTNode_T* left);
static ASTNode_T* parse_pow_3(Parser_T* p, ASTNode_T* left);

static ASTNode_T* parse_current_fn_token(Parser_T* p);

static ASTType_T* parse_type(Parser_T* p);
static ASTObj_T* parse_fn_def(Parser_T* p);

typedef struct {
    const char* operator;
    const PrefixParseFn_T pfn; 
    const InfixParseFn_T ifn; 
    const Precedence_T precedence;
} OperatorContext_T;

static const OperatorContext_T builtin_operators[] = {
    {"++",  NULL, parse_postfix, PREC_INC}, // increment
    {"+=",  NULL, parse_assignment, PREC_ASSIGN}, // add assign
    {"+",   NULL, parse_num_op, PREC_PLUS}, // add
    {"--",  NULL, parse_postfix, PREC_DEC}, // decrement
    {"-=",  NULL, parse_assignment, PREC_ASSIGN}, // subtract assign
    {"-",   parse_unary, parse_num_op, PREC_MINUS}, // subtract
    {"*=",  NULL, parse_assignment, PREC_ASSIGN}, // multiply assign
    {"*",   parse_unary, parse_num_op, PREC_MULT}, // multiply
    {"/=",  NULL, parse_assignment, PREC_ASSIGN}, // division assign
    {"/",   NULL, parse_num_op, PREC_DIV}, // division
    {"%=",  NULL, parse_assignment, PREC_ASSIGN}, // modulo assign
    {"%",   NULL, parse_num_op, PREC_MOD}, // modulo
    {"&=",  NULL, parse_assignment, PREC_ASSIGN}, // bit and assign
    {"&",   parse_unary, parse_bit_op, PREC_BIT_AND}, // bit and
    {"&&",  NULL, parse_bool_op, PREC_LOGIC_AND}, // logical and
    {"|=",  NULL, parse_assignment, PREC_ASSIGN}, // bit or assign
    {"|",   parse_lambda_lit, parse_bit_op, PREC_BIT_OR}, // bit or
    {"||",  parse_lambda_lit, parse_bool_op, PREC_LOGIC_OR}, // logical or
    {"^=",  NULL, parse_assignment, PREC_ASSIGN}, // bit xor assign
    {"^",   NULL, parse_bit_op, PREC_BIT_XOR}, // bit xor
    {"<<=", NULL, parse_assignment, PREC_ASSIGN}, // left bit shift assign
    {"<<",  NULL, parse_bit_op, PREC_BIT_SHIFT}, // left bit shift
    {">>=",  NULL, parse_assignment, PREC_ASSIGN}, // right bit shift assign
    {">>",  NULL, parse_bit_op, PREC_BIT_SHIFT}, // right bit shift
    {"~",   parse_unary, NULL, PREC_LOWEST}, // bit not
    {"|>",  NULL, parse_pipe, PREC_PIPE}, // pipe
    {"==",  NULL, parse_bool_op, PREC_EQUALS}, // equals
    {"!=",  NULL, parse_bool_op, PREC_EQUALS}, // not equals
    {"!",   parse_unary, NULL, PREC_LOWEST},  // not
    {">",   NULL, parse_bool_op, PREC_GT}, // greater than
    {">=",   NULL, parse_bool_op, PREC_GT}, // greater than or equals
    {"<",   NULL, parse_bool_op, PREC_LT}, // less than
    {"<=",   NULL, parse_bool_op, PREC_LT}, // less than or equals
    {"=",   NULL, parse_assignment, PREC_ASSIGN}, // assign
    {":",   NULL, parse_cast, PREC_CAST}, // type cast
    {"::",  parse_id, NULL, PREC_LOWEST}, // static member
    {".",   NULL, parse_member, PREC_MEMBER}, // member
    {"..",  NULL, NULL, PREC_LOWEST}, // range
    {NULL,  NULL, NULL, PREC_LOWEST}
};

static const struct { 
    PrefixParseFn_T pfn; 
    InfixParseFn_T ifn; 
    Precedence_T prec; 
} expr_parse_fns[TOKEN_EOF + 1] = {
    [TOKEN_ID]       = {parse_id, NULL, PREC_LOWEST},
    [TOKEN_INT]      = {parse_int_lit, NULL, PREC_LOWEST},
    [TOKEN_FLOAT]    = {parse_float_lit, NULL, PREC_LOWEST},
    [TOKEN_NIL]      = {parse_nil_lit, NULL, PREC_LOWEST},
    [TOKEN_TRUE]     = {parse_bool_lit, NULL, PREC_LOWEST},
    [TOKEN_FALSE]    = {parse_bool_lit, NULL, PREC_LOWEST},
    [TOKEN_CHAR]     = {parse_char_lit, NULL, PREC_LOWEST},
    [TOKEN_STRING]   = {parse_str_lit, NULL, PREC_LOWEST},
    [TOKEN_LPAREN]   = {parse_closure, parse_call, PREC_CALL}, 
    [TOKEN_LBRACKET] = {parse_array_lit, parse_index, PREC_ARRAY},   
    [TOKEN_LBRACE]   = {parse_anonymous_struct_lit, NULL, PREC_LOWEST}, 
    [TOKEN_SIZEOF]   = {parse_sizeof, NULL, PREC_X_OF},
    [TOKEN_ALIGNOF]  = {parse_alignof, NULL, PREC_X_OF},
    [TOKEN_LEN]      = {parse_len, NULL, PREC_LOWEST},
    [TOKEN_POW_2]    = {NULL, parse_pow_2, PREC_POWER},
    [TOKEN_POW_3]    = {NULL, parse_pow_3, PREC_POWER},
    [TOKEN_DOLLAR]   = {parse_hole, NULL, PREC_LOWEST},
    [TOKEN_ELSE]     = {NULL, parse_else_expr, PREC_INFIX_CALL},
    [TOKEN_CONST]    = {parse_const_expr, NULL, PREC_LOWEST},
    [TOKEN_TYPE]     = {parse_type_expr, NULL, PREC_LOWEST},
    [TOKEN_INFIX_CALL] = {NULL, parse_infix_call, PREC_INFIX_CALL},
    [TOKEN_IF]       = {parse_ternary, NULL, PREC_LOWEST},
    [TOKEN_CURRENT_FN] = {parse_current_fn_token, NULL, PREC_LOWEST},
}; 

static const ASTNodeKind_T unary_ops[CHAR_MAX] = {
    ['-'] = ND_NEG,
    ['!'] = ND_NOT,
    ['~'] = ND_BIT_NEG,
    ['&'] = ND_REF,
    ['*'] = ND_DEREF
};

static const struct {
    const char* op;
    const ASTNodeKind_T kind;
} infix_ops[TOKEN_EOF + 1] = {
    {"-", ND_SUB},
    {"+", ND_ADD},
    {"*", ND_MUL},
    {"/", ND_DIV},
    {"%", ND_MOD},
    {"==", ND_EQ},
    {"!=", ND_NE},
    {">", ND_GT},
    {">=", ND_GE},
    {"<", ND_LT},
    {"<=", ND_LE},
    {"&&", ND_AND},
    {"||", ND_OR},
    {"=", ND_ASSIGN},
    {"+=", ND_ADD},
    {"-=", ND_SUB},
    {"*=", ND_MUL},
    {"/=", ND_DIV},
    {"%=", ND_DIV},
    {"<<=", ND_LSHIFT},
    {">>=", ND_RSHIFT},
    {"^=", ND_XOR},
    {"|=", ND_BIT_OR},
    {"&=", ND_BIT_AND},
    {">>", ND_RSHIFT},
    {"<<", ND_LSHIFT},
    {"^", ND_XOR},
    {"|", ND_BIT_OR},
    {"&", ND_BIT_AND},

    // technically postfix operators, but get treated like infix ops internally
    {"++", ND_INC},
    {"--", ND_DEC},
    {NULL, 0}
};

static inline ASTNodeKind_T get_infix_op(const char* op)
{
    for(size_t i = 0; infix_ops[i].op; i++)
    {
        if(strcmp(op, infix_ops[i].op) == 0)
            return infix_ops[i].kind;
    }

    unreachable();
    return -1;
}

static inline PrefixParseFn_T get_PrefixParseFn_T(Parser_T* p, Token_T* tok)
{
    if(tok->type == TOKEN_OPERATOR)
    {
        OperatorContext_T* context = hashmap_get(p->operator_context, tok->value);
        if(context)
            return context->pfn;
        return parse_custom_prefix_operator;
    }
    else
        return expr_parse_fns[tok->type].pfn;
}

static inline InfixParseFn_T get_InfixParseFn_T(Parser_T* p, Token_T* tok)
{
    if(tok->type == TOKEN_OPERATOR)
    {
        OperatorContext_T* context = hashmap_get(p->operator_context, tok->value);
        if(context)
            return context->ifn;
        return parse_custom_infix_operator;
    }
    else
        return expr_parse_fns[tok->type].ifn;
}

static inline Precedence_T get_precedence(Parser_T* p, Token_T* tok)
{
    if(tok->type == TOKEN_OPERATOR)
    {
        OperatorContext_T* context = hashmap_get(p->operator_context, tok->value);
        if(context)
            return context->precedence;
        return PREC_CUSTOM_OPERATOR;
    }
    else
        return expr_parse_fns[tok->type].prec;
}

/////////////////////////////////
// helperfunctions             //
/////////////////////////////////

static HashMap_T* build_operator_context(void)
{
    HashMap_T* map = hashmap_init();

    for(size_t i = 0; builtin_operators[i].operator; i++)
        hashmap_put(map, (char*) builtin_operators[i].operator, (void*) &builtin_operators[i]);
    
    return map;
}

static void init_parser(Parser_T* parser, Context_T* context, ASTProg_T* ast)
{
    memset(parser, 0, sizeof(struct PARSER_STRUCT));
    parser->context = context;
    parser->root_ref = ast;
    parser->tokens = ast->tokens;
    parser->tok = ast->tokens->items[0];
    parser->operator_context = build_operator_context();
}

static void free_parser(Parser_T* p)
{
    hashmap_free((HashMap_T*) p->operator_context);
}

static inline bool streq(char* s1, char* s2)
{
    return strcmp(s1, s2) == 0;
}

static inline Token_T* parser_advance(Parser_T* p)
{
    p->tok = p->tokens->items[++p->token_i];
    if(p->tok->type == TOKEN_SEMICOLON && streq(p->tok->value, ";"))
        throw_error(p->context, ERR_SYNTAX_WARNING, p->tok, "found `;` (greek question mark) instead of `;` (semicolon)");
    return p->tok;
}

Context_T* parser_context(Parser_T* p)
{
    return p->context;
}

ASTProg_T* parser_ast(Parser_T* p)
{
    return p->root_ref;
}

inline Token_T* parser_peek(Parser_T* p, i32 level)
{
    if(level == 0)
        return p->tok;
    if(p->token_i + level >= p->tokens->size || p->token_i + level <= 0)
        return NULL;
    return p->tokens->items[p->token_i + level];
}

inline bool tok_is(Parser_T* p, TokenType_T type)
{
    return p->tok->type == type;
}

static inline bool is_operator(Token_T* tok, const char* op)
{
    return tok->type == TOKEN_OPERATOR && strcmp(tok->value, op) == 0;
}

static inline bool tok_is_operator(Parser_T* p, const char* op)
{
    return is_operator(p->tok, op);
}

Token_T* parser_consume(Parser_T* p, TokenType_T type, const char* msg)
{
    if(!tok_is(p, type))
        throw_error(p->context, ERR_SYNTAX_ERROR, p->tok, "unexpected token `%s`, %s", p->tok->value,  msg);

    return parser_advance(p);
}

static Token_T* parser_consume_operator(Parser_T* p, const char* op, const char* msg)
{
    if(!tok_is_operator(p, op))
        throw_error(p->context, ERR_SYNTAX_ERROR, p->tok, "unexpected token `%s`, %s", p->tok->value,  msg);

    return parser_advance(p);
}

static inline void parser_enable_holes(Parser_T* p)
{
    p->holes_enabled = true;
}

static inline void parser_disable_holes(Parser_T* p)
{
    p->holes_enabled = false;
}

static inline bool parser_holes_enabled(Parser_T* p)
{
    return p->holes_enabled;
}

static inline bool is_executable(ASTNode_T* n)
{
    if(n->kind == ND_CLOSURE)
        return is_executable(n->exprs->items[n->exprs->size - 1]);
    if(n->kind == ND_PIPE)
        return is_executable(n->right);
    if(n->kind == ND_TERNARY)
        return is_executable(n->if_branch) && is_executable(n->else_branch);
    return n->kind == ND_CALL || n->kind == ND_ASSIGN || n->kind == ND_INC || n->kind == ND_DEC || n->kind == ND_CAST || n->kind == ND_MEMBER || n->kind == ND_ASM;
}

static bool check_type(ASTType_T* a, ASTType_T* b)
{
    if(a->kind != b->kind)
        return false;
    
    /*if(a->callee != NULL && b->callee != NULL && strcmp(a->callee, b->callee) != 0)
        return false;
    
    if((a->callee == NULL && b->callee != NULL) || (a->callee != NULL && b->callee == NULL))
        return false;*/
    
    if(a->is_primitive != b->is_primitive)
        return false;
    
    if((a->base == NULL && b->base != NULL) || (a->base != NULL && b->base == NULL))
        return false;

    if(a->base && b->base)
        return check_type(a->base, b->base);

    return true;
}

static ASTObj_T* get_compatible_tuple(Parser_T* p, ASTType_T* tuple)
{
    for(size_t i = 0; i < p->root_ref->objs->size; i++)
    {
        ASTObj_T* obj = p->root_ref->objs->items[i];
        if(obj->kind == OBJ_TYPEDEF && str_starts_with(obj->id->callee, "__csp_tuple_") && obj->data_type->members->size == tuple->members->size)
        {
            for(size_t j = 0; j < obj->data_type->members->size; j++)
            {
                if(!check_type(((ASTNode_T*) obj->data_type->members->items[j])->data_type, ((ASTNode_T*) tuple->members->items[j])->data_type))
                    goto cont;
            }
            return obj;
        }
    cont:
        ;
    }

    return NULL;
}

/////////////////////////////////
// Parser                      //
/////////////////////////////////

i32 parser_pass(Context_T* context, ASTProg_T* ast)
{
    if(!context->flags.silent)
    {
        LOG_OK_F(COLOR_BOLD_GREEN "\33[2K\r  Compiling " COLOR_RESET " %s\n", ((File_T*) ast->files->items[0])->path);
    }

    context->total_source_lines = 0;
    for(size_t i = 0; i < ast->files->size; i++)
    {
        File_T* file = ast->files->items[i];
        context->total_source_lines += file->num_lines;
    }
    
    // initialize the parser;
    timer_start(context, "parsing");
    Parser_T parser;
    init_parser(&parser, context, ast);

    context->current_obj = &parser.cur_obj;

    // parse
    while(!tok_is(&parser, TOKEN_EOF))
    {
        switch(parser.tok->type)
        {
            case TOKEN_IMPORT:
                parser_advance(&parser);
                parser_consume(&parser, TOKEN_STRING, "expect file to import as string");
                parser_consume(&parser, TOKEN_SEMICOLON, "expect `;` after import statement");
                break;
            default: 
                parse_obj(&parser, ast->objs);
        }
    }

    context->current_obj = NULL;

    // dispose
    free_list(ast->tokens);
    free_parser(&parser);

    timer_stop(context);

    return 0;
}

/////////////////////////////////
// Identifier Parser           //
/////////////////////////////////

static ASTIdentifier_T* __parse_identifier(Parser_T* p, ASTIdentifier_T* outer, bool is_simple)
{
    bool global_scope = false;
    if(tok_is_operator(p, STATIC_MEMBER) && parser_peek(p, 1)->type == TOKEN_ID)
    {
        parser_advance(p);
        global_scope = true;
    }

    ASTIdentifier_T* id = init_ast_identifier(p->tok, p->tok->value);
    id->outer = outer;
    id->global_scope = global_scope;
    parser_consume(p, TOKEN_ID, "expect identifier");

    if(tok_is_operator(p, STATIC_MEMBER) && !is_simple && parser_peek(p, 1)->type == TOKEN_ID)
    {
        if(is_operator(parser_peek(p, 1), "<"))
           return id; // :: followed by < would be a generic in a function call 
        
        parser_advance(p);
        return __parse_identifier(p, id, false);
    }
    return id;
}

#define parse_identifier(p) __parse_identifier(p, NULL, false)
#define parse_simple_identifier(p) __parse_identifier(p, NULL, true)

/////////////////////////////////
// Datatype Parser             //
/////////////////////////////////

static ASTNode_T* parse_expr(Parser_T* p, Precedence_T prec, TokenType_T end_tok);

static ASTType_T* parse_struct_type(Parser_T* p)
{
    ASTType_T* struct_type = init_ast_type(TY_STRUCT, p->tok);
    if(tok_is(p, TOKEN_STRUCT))
        parser_consume(p, TOKEN_STRUCT, "expect `struct` keyword for struct type");
    else
    { 
        struct_type->is_union = true;
        parser_consume(p, TOKEN_UNION, "expect `union` keyword for struct type");
    }

    parser_consume(p, TOKEN_LBRACE, "expect `{` or identifier after struct keyword");
    struct_type->members = init_list();
    mem_add_list(struct_type->members);

    while(!tok_is(p, TOKEN_RBRACE) && !tok_is(p, TOKEN_EOF))
    {
        if(tok_is(p, TOKEN_EMBED))
        {
            parser_advance(p);

            ASTNode_T* member = init_ast_node(ND_EMBED_STRUCT, p->tok);
            member->data_type = parse_type(p);

            list_push(struct_type->members, member);
        }
        else
        {
            ASTNode_T* member = init_ast_node(ND_STRUCT_MEMBER, p->tok);
            member->id = parse_simple_identifier(p);
            parser_consume_operator(p, ":", "expect `:` after struct member name");
            member->data_type = parse_type(p);

            list_push(struct_type->members, member);
        }

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

    enum_type->members = init_list();
    mem_add_list(enum_type->members);

    for(i32 i = 0; !tok_is(p, TOKEN_RBRACE) && !tok_is(p, TOKEN_EOF); i++)
    {
        ASTObj_T* member = init_ast_obj(OBJ_ENUM_MEMBER, p->tok);
        member->data_type = (ASTType_T*) primitives[TY_I32];
        member->id = parse_simple_identifier(p);
        list_push(enum_type->members, member);
        member->is_constant = true;

        if(tok_is_operator(p, "="))
        {
            parser_advance(p);
            
            member->value = parse_expr(p, PREC_LOWEST, TOKEN_COMMA);
        }
        else {
            member->value = init_ast_node(ND_NOOP, member->tok);
        }

        if(!tok_is(p, TOKEN_RBRACE))
            parser_consume(p, TOKEN_COMMA, "expect `,` between enum members");
    }

    parser_consume(p, TOKEN_RBRACE, "expect `}` after enum members");
    return enum_type;
}

static ASTType_T* parse_lambda_type(Parser_T* p)
{
    ASTType_T* lambda = init_ast_type(TY_FN, p->tok);

    parser_consume(p, TOKEN_FN, "expect `fn` keyword for lambda type");

    if(tok_is_operator(p, "<"))
    {
        parser_advance(p);
        lambda->base = parse_type(p);
        parser_consume_operator(p, ">", "expect `>` after lambda return type");
    }
    else if(p->tok->value[0] == '<')
    {
        trim_first_char(p->tok->value);
        lambda->base = parse_type(p);
        parser_consume_operator(p, ">", "expect `>` after lambda return type");
    }
    else
    {
        lambda->base = (ASTType_T*) primitives[TY_VOID];
    }

    lambda->arg_types = init_list();
    mem_add_list(lambda->arg_types);

    if(tok_is(p, TOKEN_LPAREN))
    {
        parser_consume(p, TOKEN_LPAREN, "expect `(` before lambda argument types");
        

        while(!tok_is(p, TOKEN_RPAREN) && !tok_is(p, TOKEN_EOF))
        {
            list_push(lambda->arg_types, parse_type(p));

            if(!tok_is(p, TOKEN_RPAREN))
                parser_consume(p, TOKEN_COMMA, "expect `,` between lambda argument types");
        }

        parser_consume(p, TOKEN_RPAREN, "expect `)` after lambda argument types");
    }

    return lambda;
}

static ASTType_T* parse_interface_type(Parser_T* p)
{
    ASTType_T* interface = init_ast_type(TY_INTERFACE, p->tok);
    parser_consume(p, TOKEN_INTERFACE, "expect `interface` keyword for interface type");
    parser_consume(p, TOKEN_LBRACE, "expect `{` after `interface`");

    interface->func_decls = init_list();
    mem_add_list(interface->func_decls);

    while(!tok_is(p, TOKEN_RBRACE) && !tok_is(p, TOKEN_EOF))
    {
        list_push(interface->func_decls, parse_fn_def(p));

        if(!tok_is(p, TOKEN_RBRACE))
            parser_consume(p, TOKEN_COMMA, "expect `,` between interface functions");
    }

    parser_consume(p, TOKEN_RBRACE, "expect `}` after interface body");

    return interface;
}

static ASTObj_T* parser_generate_tuple_type(Parser_T* p, ASTType_T* tuple)
{
    ASTObj_T* existing_tydef = get_compatible_tuple(p, tuple);
    if(existing_tydef)
    {
        tuple->kind = TY_UNDEF;
        tuple->id = existing_tydef->id;

        return existing_tydef;
    }
    else
    {
        ASTObj_T* tydef = init_ast_obj(OBJ_TYPEDEF, tuple->tok);
        tydef->data_type = mem_malloc(sizeof(struct AST_TYPE_STRUCT));
        *tydef->data_type = *tuple;

        char* id = calloc(35, sizeof(char));
        sprintf(id, "__csp_tuple_%lu__", p->cur_tuple_id++);
        mem_add_ptr(id);

        tydef->id = init_ast_identifier(tuple->tok, id);

        list_push(p->root_ref->objs, tydef);
        tuple->kind = TY_UNDEF;
        tuple->id = tydef->id;

        return tydef;
    }
}

static ASTType_T* parse_type(Parser_T* p)
{
    ASTType_T* type = get_primitive_type(p->tok->value);
    if(type)
        parser_advance(p);
    else
    {
        switch(p->tok->type)
        {
            case TOKEN_CONST: 
                parser_advance(p);
                type = parse_type(p);
                type->is_constant = true;
                return type;

            case TOKEN_LPAREN:
                parser_advance(p);
                type = parse_type(p);
                parser_consume(p, TOKEN_RPAREN, "expect closing `)` after data type");
                break;
            case TOKEN_FN:
                type = parse_lambda_type(p);
                break;
            case TOKEN_UNION:
            case TOKEN_STRUCT:
                type = parse_struct_type(p);
                break;
            case TOKEN_INTERFACE:
                type = parse_interface_type(p);
                break;
            case TOKEN_ENUM:
                type = parse_enum_type(p);
                break;
            case TOKEN_OPERATOR: {
                if(tok_is_operator(p, STATIC_MEMBER))
                {
                    type = init_ast_type(TY_UNDEF, p->tok);
                    type->id = parse_identifier(p);
                    break;
                }
                
                size_t num_ptrs = strlen(p->tok->value);
                if(str_count_char(p->tok->value, '&') != num_ptrs || !num_ptrs)
                    throw_error(p->context, ERR_SYNTAX_ERROR, p->tok, "expect `&`, `&&` or similar");
                parser_advance(p);
                type = parse_type(p);
                do {
                    ASTType_T* base = type;
                    type = init_ast_type(TY_PTR, p->tok);
                    type->base = base;
                } while(--num_ptrs);
            } break;
            case TOKEN_LBRACE:
                type = init_ast_type(TY_STRUCT, p->tok);
                type->members = init_list();
                mem_add_list(type->members);
                parser_advance(p);

                for(size_t i = 0; !tok_is(p, TOKEN_RBRACE) && !tok_is(p, TOKEN_EOF); i++)
                {
                    ASTNode_T* member = init_ast_node(ND_STRUCT_MEMBER, p->tok);
                    member->data_type = parse_type(p);

                    char* id = calloc(22, sizeof(char));
                    mem_add_ptr(id);
                    sprintf(id, "_%lu", i);

                    member->id = init_ast_identifier(p->tok, id);

                    list_push(type->members, member);
                    if(!tok_is(p, TOKEN_RBRACE))
                        parser_consume(p, TOKEN_COMMA, "expect `,` between tuple argument types");
                }
                parser_consume(p, TOKEN_RBRACE, "expect `}` after tuple argument types");
                parser_generate_tuple_type(p, type);
                break;
            case TOKEN_TYPEOF:
                type = init_ast_type(TY_TYPEOF, p->tok);
                parser_advance(p);
                type->num_indices_node = parse_expr(p, PREC_X_OF, TOKEN_SEMICOLON);
                break;
            default:
                type = init_ast_type(TY_UNDEF, p->tok);
                type->id = parse_identifier(p);
                break;
        }
    }

parse_array_ty:
    switch(p->tok->type)
    {
    case TOKEN_LBRACKET: // normal arrays and variable length arrays (VLAs)
        {
            ASTType_T* arr_type = init_ast_type(TY_VLA, p->tok);
            parser_advance(p);
            if(!tok_is(p, TOKEN_RBRACKET))
            {
                arr_type->kind = TY_ARRAY;
                arr_type->num_indices_node = parse_expr(p, PREC_LOWEST, TOKEN_RBRACKET);
            }
            parser_consume(p, TOKEN_RBRACKET, "expect `]` after array type");
            arr_type->base = type;
            type = arr_type;
        }
        // repeat for arrays of arrays
        goto parse_array_ty;

    case TOKEN_C_ARRAY: // legacy C-like arrays for compatibility
        {   
            ASTType_T* arr_type = init_ast_type(TY_C_ARRAY, p->tok);
            parser_advance(p);
            parser_consume(p, TOKEN_LBRACKET, "expect `[` after `'c`");
            arr_type->num_indices_node = parse_expr(p, PREC_LOWEST, TOKEN_RBRACKET);
            parser_consume(p, TOKEN_RBRACKET, "expect `]` after array length");
            arr_type->base = type;
            type = arr_type;
        }
        // repeat for arrays of arrays
        goto parse_array_ty;
    
    default:
        return type;
    }
}

/////////////////////////////////
// Definition & Obj Parser     //
/////////////////////////////////

static ASTObj_T* parse_global(Parser_T* p);

static ASTObj_T* parse_typedef(Parser_T* p)
{
    ASTObj_T* tydef = init_ast_obj(OBJ_TYPEDEF, p->tok);
    parser_consume(p, TOKEN_TYPE, "expect `type` keyword for typedef");

    ASTObj_T* last_obj = p->cur_obj;
    p->cur_obj = tydef;

    tydef->id = parse_simple_identifier(p);
    parser_consume_operator(p, ":", "expect `:` after type name");

    tydef->data_type = parse_type(p);

    parser_consume(p, TOKEN_SEMICOLON, "expect `;` after type definition");

    p->cur_obj = last_obj;
    return tydef;
}

static ASTObj_T* parse_extern_def(Parser_T *p, bool is_extern_c)
{
    switch(p->tok->type)
    {
        case TOKEN_CONST:
        case TOKEN_LET:
        {
            ASTObj_T* ext_var = parse_global(p);
            ext_var->is_extern = true;
            ext_var->is_extern_c = is_extern_c;

            if(ext_var->value)
                throw_error(p->context, ERR_SYNTAX_WARNING, ext_var->value->tok, "cannot set a value to an extern variable");
            return ext_var;
        }
        case TOKEN_OPERATOR_KW:
        case TOKEN_FN:
        {
            ASTObj_T* ext_fn = parse_fn_def(p);
            if(tok_is(p, TOKEN_SEMICOLON))
                parser_advance(p);
            ext_fn->is_extern = true;
            ext_fn->is_extern_c = is_extern_c;

            return ext_fn;
        }
        case TOKEN_LBRACKET:
        {
            List_T* dummy = init_list();
            parse_directives(p, dummy);

            if(dummy->size)
            {
                ASTObj_T* obj = dummy->items[0];
                if(obj->kind == OBJ_FUNCTION || obj->kind == OBJ_GLOBAL)
                {
                    free_list(dummy);
                    obj->is_extern = true;
                    obj->is_extern_c = is_extern_c;
                    return obj;
                }
            }
            free_list(dummy);
        } // fall through
        default:
            throw_error(p->context, ERR_SYNTAX_ERROR, p->tok, "expect function or variable declaration");
            break;
    }

    // satisfy -Wall
    return NULL;
}

static void parse_extern(Parser_T* p, List_T* objs)
{
    parser_advance(p);

    bool extern_c = tok_is(p, TOKEN_STRING) && (streq(p->tok->value, "C") || streq(p->tok->value, "c"));
    if(extern_c)
        parser_advance(p);
    else if(tok_is(p, TOKEN_STRING))
        throw_error(p->context, ERR_SYNTAX_ERROR, p->tok, "invalid `extern` parameter `\"%s\"`, expect `\"C\"` or `{`", p->tok->value);

    if(tok_is(p, TOKEN_LBRACE)) {
        parser_advance(p);
        while(!tok_is(p, TOKEN_RBRACE) && !tok_is(p, TOKEN_EOF))
        {
            list_push(objs, parse_extern_def(p, extern_c));  
        }

        parser_consume(p, TOKEN_RBRACE, "expect `}` after extern function/variable definitions");
        return;
    }

    list_push(objs, parse_extern_def(p, extern_c));    
}

List_T* parse_argument_list(Parser_T* p, TokenType_T end_tok, ASTIdentifier_T** variadic_id)
{
    List_T* arg_list = init_list();

    while(p->tok->type != end_tok)
    {
        if(parser_peek(p, 2)->type == TOKEN_VA_LIST)
        {
            (*variadic_id) = parse_simple_identifier(p);
            parser_consume_operator(p, ":", "expect `:` after argument name");
            parser_advance(p);

            if(!tok_is(p, end_tok))
                throw_error(p->context, ERR_SYNTAX_ERROR, p->tok, "expect `...` to be the last function argument");

            break;
        }
        else 
        {
            ASTObj_T* arg = init_ast_obj(OBJ_FN_ARG, p->tok);
            arg->id = parse_simple_identifier(p);
            parser_consume_operator(p, ":", "expect `:` after argument name");

            arg->data_type = parse_type(p);
            list_push(arg_list, arg);

            if(p->tok->type != end_tok)
                parser_consume(p, TOKEN_COMMA, "expect `,` between arguments");
        }
    }

    return arg_list;
}

static ASTNode_T* parse_stmt(Parser_T* p, bool needs_semicolon);

static ASTObj_T* parse_fn_def(Parser_T* p)
{
    ASTObj_T* fn = init_ast_obj(OBJ_FUNCTION, p->tok);
    bool is_operator_define = false;

    if(tok_is(p, TOKEN_OPERATOR_KW))
    {  
        parser_advance(p);
        fn->id = init_ast_identifier(p->tok, p->tok->value);
        if(p->tok->type != TOKEN_OPERATOR)
            throw_error(p->context, ERR_SYNTAX_ERROR, p->tok, "expect operator after `operator`");
        if(hashmap_get(p->operator_context, p->tok->value))
            throw_error(p->context, ERR_REDEFINITION, p->tok, "redefinition of built-in operator `%s`", p->tok->value);
        parser_advance(p);
        is_operator_define = true;
    }
    else 
    {
        parser_consume(p, TOKEN_FN, "expect `fn` keyword for a function definition");
        fn->id = parse_simple_identifier(p);
    }

    parser_consume(p, TOKEN_LPAREN, "expect `(` after function name");
    
    ASTIdentifier_T* va_id = NULL;
    fn->args = parse_argument_list(p, TOKEN_RPAREN, &va_id);
    mem_add_list(fn->args);

    if(is_operator_define && fn->args->size != 1 && fn->args->size != 2)
        throw_error(p->context, ERR_SYNTAX_ERROR_UNCR, fn->tok, "`operator` functions can only take one or two arguments, got `%zu`", fn->args->size);

    if(va_id)
    {
        fn->va_area = init_ast_obj(OBJ_LOCAL, fn->tok);
        fn->va_area->id = va_id;
        fn->va_area->data_type = init_ast_type(TY_C_ARRAY, fn->tok);
        fn->va_area->data_type->num_indices = 136;
        fn->va_area->data_type->base = (ASTType_T*) primitives[TY_U8];
    }

    parser_consume(p, TOKEN_RPAREN, "expect `)` after function arguments");

    if(tok_is_operator(p, ":"))
    {
        parser_advance(p);
        fn->return_type = parse_type(p);
    } else
        fn->return_type = (ASTType_T*) primitives[TY_VOID];

    fn->data_type = init_ast_type(TY_FN, fn->tok);
    fn->data_type->base = fn->return_type;
    fn->data_type->is_constant = true;
    fn->data_type->arg_types = init_list();
    fn->data_type->size = PTR_S;
    for(size_t i = 0; i < fn->args->size; i++)
        list_push(fn->data_type->arg_types, ((ASTObj_T*) fn->args->items[i])->data_type);
    mem_add_list(fn->data_type->arg_types);
    fn->data_type->is_variadic = fn->va_area != NULL;

    if(p->context->ct == CT_ASM)
        fn->alloca_bottom = &alloca_bottom;

    return fn;
}

static ASTObj_T* parse_fn(Parser_T* p)
{
    ASTObj_T* fn = parse_fn_def(p);

    ASTObj_T* last_obj = p->cur_obj;
    p->cur_obj = fn;
    
    if(tok_is_operator(p, "="))
    {
        fn->body = init_ast_node(ND_RETURN, p->tok);
        parser_advance(p);

        fn->body->return_val = parse_expr(p, PREC_LOWEST, TOKEN_SEMICOLON);
        parser_consume(p, TOKEN_SEMICOLON, "expect `;` after short function body");
    }
    else
        fn->body = parse_stmt(p, true);

    if(p->context->ct == CT_ASM)
    {
        fn->objs = init_list();
        mem_add_list(fn->objs);
        collect_locals(fn->body, fn->objs);
    }

    p->cur_obj = last_obj;
    
    return fn;
}

static ASTObj_T* parse_global(Parser_T* p)
{
    ASTObj_T* global = init_ast_obj(OBJ_GLOBAL, p->tok);
    if(p->tok->type == TOKEN_LET)
        parser_advance(p);
    else if(p->tok->type == TOKEN_CONST)
    {
        global->is_constant = true;
        parser_advance(p);
    }
    else
        throw_error(p->context, ERR_SYNTAX_ERROR, p->tok, "expect `let` keyword for variable definition");
    
    global->id = parse_simple_identifier(p);

    parser_consume_operator(p, ":", "expect `:` after variable name");
    global->data_type = parse_type(p);
    if(tok_is_operator(p, "="))
    {
        parser_advance(p);
        global->value = parse_expr(p, PREC_LOWEST, TOKEN_SEMICOLON);
    }
    
    parser_consume(p, TOKEN_SEMICOLON, "expect `;` after variable definition");
    return global;
}

static ASTObj_T* find_namespace(List_T* objs, char* callee)
{
    for(size_t i = 0; i < objs->size; i++)
    {
        ASTObj_T* obj = objs->items[i];
        if(obj->kind == OBJ_NAMESPACE && strcmp(obj->id->callee, callee) == 0)
            return obj;
    }
    return NULL;
}

static void parse_namespace(Parser_T* p, List_T* objs)
{
    Token_T* tok = p->tok;
    parser_advance(p); // skip the "namespace" token
    ASTIdentifier_T* id = parse_simple_identifier(p);


    // if there is already a namespace with this name in the current scope, add the new objs to it rather than creating a new namespace
    ASTObj_T* namespace = find_namespace(objs, id->callee);
    if(!namespace)
    {
        namespace = init_ast_obj(OBJ_NAMESPACE, tok);
        namespace->id = id;
        list_push(objs, namespace);

        // initialize the namespace's object list
        namespace->objs = init_list();
        mem_add_list(namespace->objs);
    }

    ASTObj_T* last_obj = p->cur_obj;
    p->cur_obj = namespace;
        
    // if the namespace has a { directly after its name, it exists in the current scope
    parser_consume(p, TOKEN_LBRACE, "expect `{` after namespace declaration");

    while(!tok_is(p, TOKEN_RBRACE) && !tok_is(p, TOKEN_EOF))
    {
        parse_obj(p, namespace->objs);
    }

    parser_consume(p, TOKEN_RBRACE, "expect `}` at end of namespace");

    for(size_t i = 0; i < namespace->objs->size; i++)
    {
        ASTObj_T* obj = namespace->objs->items[i];
        obj->id->outer = namespace->id;
    }

    p->cur_obj = last_obj;
}

void parse_obj(Parser_T* p, List_T* obj_list)
{
    switch(p->tok->type)
    {
        case TOKEN_TYPE:
            list_push(obj_list, parse_typedef(p));
            break;
        case TOKEN_CONST:
        case TOKEN_LET:
            list_push(obj_list, parse_global(p));
            break;
        case TOKEN_FN:
        case TOKEN_OPERATOR_KW:
            list_push(obj_list, parse_fn(p));
            break;
        case TOKEN_EXTERN:  
            parse_extern(p, obj_list);
            break;
        case TOKEN_NAMESPACE:
            parse_namespace(p, obj_list);
            break;
        case TOKEN_LBRACKET:
            parse_directives(p, obj_list);
            break;
        default:
            throw_error(p->context, ERR_SYNTAX_ERROR, p->tok, "unexpected token `%s`, expect [import, type, let, const, fn]", p->tok->value);
    }

    
}

/////////////////////////////////
// Statement Parser            //
/////////////////////////////////

static ASTNode_T* parse_block(Parser_T* p)
{
    ASTNode_T* block = init_ast_node(ND_BLOCK, p->tok);
    block->locals = init_list();
    block->stmts = init_list();

    parser_consume(p, TOKEN_LBRACE, "expect `{` at the beginning of a block statement");

    ASTNode_T* prev_block = p->cur_block;
    p->cur_block = block;

    while(p->tok->type != TOKEN_RBRACE)
        list_push(block->stmts, parse_stmt(p, true));
    
    p->cur_block = prev_block;

    parser_consume(p, TOKEN_RBRACE, "expect `}` at the end of a block statement");

    mem_add_list(block->locals);
    mem_add_list(block->stmts);

    return block;
}

static ASTNode_T* parse_return(Parser_T* p, bool needs_semicolon)
{
    ASTNode_T* ret = init_ast_node(ND_RETURN, p->tok);

    parser_consume(p, TOKEN_RETURN, "expect `ret` or `<-` to return from function");

    if(!tok_is(p, TOKEN_SEMICOLON))
    {
        if((p->cur_obj && p->cur_obj->return_type->kind == TY_VOID))
            throw_error(p->context, ERR_TYPE_ERROR_UNCR, ret->tok, "cannot return value from function with type `void`, expect `;`");
        ret->return_val = parse_expr(p, PREC_LOWEST, TOKEN_SEMICOLON);
    }
    if(needs_semicolon)
        parser_consume(p, TOKEN_SEMICOLON, "expect `;` after return statement");

    return ret;
}

static ASTNode_T* parse_if(Parser_T* p, bool needs_semicolon)
{
    ASTNode_T* if_stmt = init_ast_node(ND_IF, p->tok);

    parser_consume(p, TOKEN_IF, "expect `if` keyword for an if statement");

    if_stmt->condition = parse_expr(p, PREC_LOWEST, TOKEN_EOF);
    if_stmt->if_branch = parse_stmt(p, needs_semicolon);

    if(tok_is(p, TOKEN_ELSE))
    {
        parser_advance(p);
        if_stmt->else_branch = parse_stmt(p, needs_semicolon);
    }

    return if_stmt;
}

static ASTNode_T* parse_loop(Parser_T* p, bool needs_semicolon)
{
    ASTNode_T* loop = init_ast_node(ND_LOOP, p->tok);

    parser_consume(p, TOKEN_LOOP, "expect `loop` keyword for a endless loop");

    loop->body = parse_stmt(p, needs_semicolon);

    return loop;
}

static ASTNode_T* parse_while(Parser_T* p, bool needs_semicolon)
{
    ASTNode_T* loop = init_ast_node(ND_WHILE, p->tok);

    parser_consume(p, TOKEN_WHILE, "expect `while` for a while loop statement");

    loop->condition = parse_expr(p, PREC_LOWEST, TOKEN_SEMICOLON);
    loop->body = parse_stmt(p, needs_semicolon);

    return loop;
}

static ASTNode_T* parse_for_range(Parser_T* p, ASTNode_T* stmt, bool needs_semicolon)
{
    stmt->kind = ND_FOR_RANGE;
    parser_consume_operator(p, "..", "expect `..` after first for loop expression");

    stmt->left = stmt->init_stmt->expr;
    stmt->right = parse_expr(p, PREC_LOWEST, TOKEN_SEMICOLON);

    stmt->body = parse_stmt(p, needs_semicolon);

    return stmt;
}

static ASTNode_T* parse_for(Parser_T* p, bool needs_semicolon)
{
    ASTNode_T* loop = init_ast_node(ND_FOR, p->tok);

    parser_consume(p, TOKEN_FOR, "expect `for` for a for loop statement");

    loop->locals = init_list();
    mem_add_list(loop->locals);

    ASTNode_T* prev_block = p->cur_block;
    p->cur_block = loop;

    if(!tok_is(p, TOKEN_SEMICOLON))
    {
        if(tok_is(p, TOKEN_LET))
        {
            ASTNode_T* init_stmt = parse_stmt(p, true);
            if(init_stmt->kind != ND_EXPR_STMT)
                throw_error(p->context, ERR_SYNTAX_ERROR, init_stmt->tok, "can only have expression-like statements in for-loop initializer");
            loop->init_stmt = init_stmt;
        }
        else {
            ASTNode_T* init_stmt = init_ast_node(ND_EXPR_STMT, p->tok);
            init_stmt->expr = parse_expr(p, PREC_LOWEST, TOKEN_SEMICOLON);
            loop->init_stmt = init_stmt;

            if(tok_is_operator(p, ".."))
                return parse_for_range(p, loop, needs_semicolon);
            parser_consume(p, TOKEN_SEMICOLON, "expect `;` after for-loop initializer");
        }
    } 
    else
        parser_advance(p);
    
    if(!tok_is(p, TOKEN_SEMICOLON))
        loop->condition = parse_expr(p, PREC_LOWEST, TOKEN_SEMICOLON);
    parser_advance(p);

    if(!tok_is(p, TOKEN_SEMICOLON))
        loop->expr = parse_expr(p, PREC_LOWEST, TOKEN_SEMICOLON);
    parser_advance(p);

    loop->body = parse_stmt(p, needs_semicolon);

    p->cur_block = prev_block;

    return loop; 
}

static ASTNode_T* parse_case(Parser_T* p)
{
    ASTNode_T* case_stmt = init_ast_node(ND_CASE, p->tok);

    switch(p->tok->type) {
    case TOKEN_UNDERSCORE:
        parser_advance(p);
        case_stmt->is_default_case = true;
        break;
    case TOKEN_OPERATOR:
        if(tok_is_operator(p, "<"))
        {
            case_stmt->mode = ND_LT;
            parser_advance(p);
        }
        else if(tok_is_operator(p, "<="))
        {
            case_stmt->mode = ND_LE;
            parser_advance(p);
        }
        else if(tok_is_operator(p, ">"))
        {
            case_stmt->mode = ND_GT;
            parser_advance(p);
        }
        else if(tok_is_operator(p, ">="))
        {
            case_stmt->mode = ND_GE;
            parser_advance(p);
        }
     
        // fall through
    default:
        if(!case_stmt->mode)
            case_stmt->mode = ND_EQ;
        case_stmt->condition = parse_expr(p, PREC_LOWEST, TOKEN_ARROW);
    }

    parser_consume(p, TOKEN_ARROW, "expect `=>` after case condition");
    case_stmt->body = parse_stmt(p, true);

    return case_stmt;
}

static ASTNode_T* parse_type_case(Parser_T* p)
{
    ASTNode_T* case_stmt = init_ast_node(ND_CASE_TYPE, p->tok);

    if(tok_is(p, TOKEN_UNDERSCORE))
    {
        parser_advance(p);
        case_stmt->is_default_case = true;
    }
    else
        case_stmt->data_type = parse_type(p);
    
    parser_consume(p, TOKEN_ARROW, "expect `=>` after case condition");
    case_stmt->body = parse_stmt(p, true);

    return case_stmt;
}

static ASTNode_T* parse_type_match(Parser_T* p, ASTNode_T* match)
{
    parser_advance(p);
    parser_advance(p);
    parser_consume(p, TOKEN_RPAREN, "expect `)` after `type`");

    match->kind = ND_MATCH_TYPE;
    match->data_type = parse_type(p);

    parser_consume(p, TOKEN_LBRACE, "expect `{` after match condition");

    while(!tok_is(p, TOKEN_RBRACE) && !tok_is(p, TOKEN_EOF))
    {
        ASTNode_T* case_stmt = parse_type_case(p);

        if(case_stmt->is_default_case)
        {
            if(match->default_case)
                throw_error(p->context, ERR_REDEFINITION, p->tok, "redefinition of default case `_`.");
            
            match->default_case = case_stmt;
            continue;
        }

        list_push(match->cases, case_stmt);
    }

    parser_consume(p, TOKEN_RBRACE, "expect `}` after match condition");
    return match;
}

static ASTNode_T* parse_match(Parser_T* p)
{
    ASTNode_T* match = init_ast_node(ND_MATCH, p->tok);
    match->cases = init_list();
    match->default_case = NULL;
    mem_add_list(match->cases);

    parser_consume(p, TOKEN_MATCH, "expect `match` keyword to match an expression");

    if(tok_is(p, TOKEN_LPAREN) && parser_peek(p, 1)->type == TOKEN_TYPE)
        return parse_type_match(p, match);

    match->condition = parse_expr(p, PREC_LOWEST, TOKEN_LBRACE);
    
    parser_consume(p, TOKEN_LBRACE, "expect `{` after match condition");

    while(!tok_is(p, TOKEN_RBRACE) && !tok_is(p, TOKEN_EOF))
    {
        ASTNode_T* case_stmt = parse_case(p);

        if(case_stmt->is_default_case)
        {
            if(match->default_case)
                throw_error(p->context, ERR_REDEFINITION, p->tok, "redefinition of default case `_`.");

            match->default_case = case_stmt;
            continue;
        }

        list_push(match->cases, case_stmt);
    }

    parser_consume(p, TOKEN_RBRACE, "expect `}` after match condition");
    return match;
}

static ASTNode_T* parse_expr_stmt(Parser_T* p, bool needs_semicolon)
{
    ASTNode_T* stmt = init_ast_node(ND_EXPR_STMT, p->tok);
    stmt->expr = parse_expr(p, PREC_LOWEST, TOKEN_SEMICOLON);

    if(!is_executable(stmt->expr))
        throw_error(p->context, ERR_SYNTAX_ERROR_UNCR, stmt->expr->tok, "cannot treat `%s` as a statement, expect function call, assignment or similar", stmt->expr->tok->value);
    if(needs_semicolon)
        parser_consume(p, TOKEN_SEMICOLON, "expect `;` after expression statement");
    return stmt;
}

static ASTNode_T* parse_local(Parser_T* p)
{
    ASTObj_T* local = init_ast_obj(OBJ_LOCAL, p->tok);
    if(p->tok->type == TOKEN_LET)
        parser_advance(p);
    else if(p->tok->type == TOKEN_CONST)
    {
        local->is_constant = true;
        parser_advance(p);
    }
    else
        throw_error(p->context, ERR_SYNTAX_ERROR, p->tok, "expect `let` keyword for variable definition");
    
    ASTNode_T* id = init_ast_node(ND_ID, p->tok);
    local->id = parse_simple_identifier(p);
    id->id = local->id;
    
    ASTNode_T* value = NULL;

    if(tok_is_operator(p, ":"))
    {
        parser_consume_operator(p, ":", "expect `:` after variable name");

        local->data_type = parse_type(p);

        if(tok_is_operator(p, "="))
        {
            value = init_ast_node(ND_ASSIGN, p->tok);
            parser_advance(p);
            value->left = id; 
            value->right = parse_expr(p, PREC_LOWEST, TOKEN_SEMICOLON);
        }
    }
    else
    {
        value = init_ast_node(ND_ASSIGN, p->tok);
        parser_consume_operator(p, "=", "expect assignment `=` after typeless variable declaration");
        value->left = id;
        value->right = parse_expr(p, PREC_LOWEST, TOKEN_SEMICOLON);
    }

    if(!value)
        value = init_ast_node(ND_NOOP, p->tok);
    else
    {
        value->referenced_obj = local;
        value->is_initializing = true;
        //value->right->is_assigning = true;
    }

    local->value = value->right;

    parser_consume(p, TOKEN_SEMICOLON, "expect `;` after variable definition");
                                                            // ND_FOR only for the for loop initializer
    if(!p->cur_block || (p->cur_block->kind != ND_BLOCK && p->cur_block->kind != ND_FOR))
        throw_error(p->context, ERR_SYNTAX_ERROR, local->tok, "cannot define a local variable outside a block statement");
    list_push(p->cur_block->locals, local);

    return value;
}

static ASTNode_T* parse_break(Parser_T* p, bool needs_semicolon)
{
    ASTNode_T* break_stmt = init_ast_node(ND_BREAK, p->tok);
    parser_consume(p, TOKEN_BREAK, "expect `break` keyword");
   
    if(needs_semicolon) 
        parser_consume(p, TOKEN_SEMICOLON, "expect `;` after break statement");

    return break_stmt;
}

static ASTNode_T* parse_continue(Parser_T* p, bool needs_semicolon)
{
    ASTNode_T* continue_stmt = init_ast_node(ND_CONTINUE, p->tok);
    parser_consume(p, TOKEN_CONTINUE, "expect `continue` keyword");

    if(needs_semicolon)
        parser_consume(p, TOKEN_SEMICOLON, "expect `;` after continue statement");

    return continue_stmt;
}

static ASTNode_T* parse_with(Parser_T* p, bool needs_semicolon)
{
    ASTNode_T* with_stmt = init_ast_node(ND_WITH, p->tok);
    parser_consume(p, TOKEN_WITH, "expect `with` keyword");

    ASTObj_T* var = with_stmt->obj = init_ast_obj(OBJ_LOCAL, p->tok);
    var->id = parse_simple_identifier(p);
    if(tok_is_operator(p, ":"))
    {
        parser_advance(p);
        var->data_type = parse_type(p);
    }
    
    ASTNode_T* assignment = var->value = init_ast_node(ND_ASSIGN, p->tok);
    assignment->left = init_ast_node(ND_ID, var->id->tok);
    assignment->left->id = var->id;
    assignment->left->referenced_obj = var;
    assignment->is_initializing = true;
    assignment->referenced_obj = var;

    parser_consume_operator(p, "=", "expect `=` after variable initializer");
    assignment->right = parse_expr(p, PREC_LOWEST, TOKEN_LBRACE);

    with_stmt->condition = assignment;
    with_stmt->if_branch = parse_stmt(p, needs_semicolon);
    if(tok_is(p, TOKEN_ELSE))
    {
        parser_advance(p);
        with_stmt->else_branch = parse_stmt(p, needs_semicolon);
    }

    return with_stmt;
}

static ASTNode_T* parse_do(Parser_T* p, bool needs_semicolon)
{
    parser_consume(p, TOKEN_DO, "expect `do` keyword");

    ASTNode_T* body = parse_stmt(p, false);

    ASTNode_T* do_stmt = NULL;
    switch(p->tok->type) 
    {
        case TOKEN_UNLESS:
            do_stmt = init_ast_node(ND_DO_UNLESS, p->tok);
            break;
        case TOKEN_WHILE:
            do_stmt = init_ast_node(ND_DO_WHILE, p->tok);
            break;
        default:
            throw_error(p->context, ERR_SYNTAX_ERROR, p->tok, "expect either `while` or `unless` after `do`-stmt body");
    }

    parser_advance(p);
    do_stmt->body = body;
    do_stmt->condition = parse_expr(p, PREC_LOWEST, TOKEN_SEMICOLON);
    parser_consume(p, TOKEN_SEMICOLON, do_stmt->kind == ND_DO_UNLESS 
        ? "expect `;` after do-unless condition" 
        : "expect `;` after do-while condition"
    );

    return do_stmt;
}

static ASTNode_T* parse_defer(Parser_T* p, bool needs_semicolon)
{
    ASTNode_T* defer = init_ast_node(ND_DEFER, p->tok);
    parser_consume(p, TOKEN_DEFER, "expect `defer` for defer statement");

    defer->body = parse_stmt(p, needs_semicolon);
    
    return defer;
}

static ASTNode_T* parse_using(Parser_T* p, bool needs_semicolon)
{
    ASTNode_T* using = init_ast_node(ND_USING, p->tok);
    parser_consume(p, TOKEN_USING, "expect `using`");

    using->ids = init_list();
    mem_add_list(using->ids);

    if(tok_is(p, TOKEN_COMMA))
        throw_error(p->context, ERR_SYNTAX_ERROR, p->tok, "expect identifier after `using`");

    do {
        if(tok_is(p, TOKEN_COMMA))
            parser_advance(p);
        list_push(using->ids, parse_identifier(p));
    } while(tok_is(p, TOKEN_COMMA));

    if(tok_is(p, TOKEN_FOR))
    {
        parser_advance(p);
        using->body = parse_stmt(p, needs_semicolon);
    }
    else if(needs_semicolon)
        parser_consume(p, TOKEN_SEMICOLON, "expect `;` after identifiers");  

    return using;
}

static ASTNode_T* parse_extern_c_block(Parser_T* p, bool needs_semicolon)
{
    ASTNode_T* extern_block = init_ast_node(ND_EXTERN_C_BLOCK, p->tok);
    parser_advance(p);

    if(!tok_is(p, TOKEN_STRING))
        throw_error(p->context, ERR_SYNTAX_ERROR, p->tok, "unexpected token `%s`, expect string literal", p->tok->value);
    if(!streq(p->tok->value, "c") && !streq(p->tok->value, "C"))
        throw_error(p->context, ERR_UNDEFINED, p->tok, "undefined `extern` mode `\"%s\"`, expect`\"C\"`", p->tok->value);
    parser_advance(p);

    parser_consume(p, TOKEN_LPAREN, "expect `(` after `extern \"C\"`");
    extern_block->body = parse_str_lit(p);
    parser_consume(p, TOKEN_RPAREN, "expect `)` after `extern \"C\" (\"...\"`");

    parser_consume(p, TOKEN_SEMICOLON, "expect `;` after `extern \"C\"` block");

    return extern_block;
}

static ASTNode_T* parse_inline_asm(Parser_T* p, bool needs_semicolon)
{
    ASTNode_T* asm_stmt = init_ast_node(ND_ASM, p->tok);
    asm_stmt->args = init_list();
    mem_add_list(asm_stmt->args);
    parser_advance(p);

    while(!tok_is(p, TOKEN_SEMICOLON))
    {
        switch (p->tok->type) {
            case TOKEN_STRING:
                list_push(asm_stmt->args, parse_str_lit(p));
                break;
            case TOKEN_INT:
                list_push(asm_stmt->args, parse_int_lit(p));
                break;
            case TOKEN_ID:
                list_push(asm_stmt->args, parse_id(p));
                break;
            case TOKEN_OPERATOR:
                if(tok_is_operator(p, "&"))
                {
                    parser_advance(p);
                    ASTNode_T* id = parse_id(p);
                    id->output = true;
                    list_push(asm_stmt->args, id);
                    break;
                }
                else if(tok_is_operator(p, STATIC_MEMBER))
                {
                    list_push(asm_stmt->args, parse_id(p));
                    break;
                }
                // fall through
            default:
                throw_error(p->context, ERR_SYNTAX_ERROR, p->tok, "unexpected token `%s` in `asm` statement", p->tok->value);
        }
    }

    if(needs_semicolon)
        parser_consume(p, TOKEN_SEMICOLON, "expect `;` after `asm` statement");

    return asm_stmt;
}

static ASTNode_T* parse_stmt(Parser_T* p, bool needs_semicolon)
{
    switch(p->tok->type)
    {
        case TOKEN_LBRACE:
            return parse_block(p);
        case TOKEN_RETURN:
            return parse_return(p, needs_semicolon);
        case TOKEN_IF:
            return parse_if(p, needs_semicolon);
        case TOKEN_LOOP:
            return parse_loop(p, needs_semicolon);
        case TOKEN_FOR:
            return parse_for(p, needs_semicolon);
        case TOKEN_WHILE:
            return parse_while(p, needs_semicolon);
        case TOKEN_MATCH:
            return parse_match(p);
        case TOKEN_WITH:
            return parse_with(p, needs_semicolon);
        case TOKEN_DO:
            return parse_do(p, needs_semicolon);
        case TOKEN_ASM:
            return parse_inline_asm(p, needs_semicolon);
        case TOKEN_CONST:
        case TOKEN_LET:
            {
                ASTNode_T* assignment = parse_local(p);
                if(assignment->kind == ND_NOOP)
                    return assignment;
                ASTNode_T* stmt = init_ast_node(ND_EXPR_STMT, assignment->tok);
                stmt->expr = assignment;
                return stmt;
            }
        case TOKEN_BREAK:
            return parse_break(p, needs_semicolon);
        case TOKEN_CONTINUE:
            return parse_continue(p, needs_semicolon);
        case TOKEN_DEFER:
            return parse_defer(p, needs_semicolon);
        case TOKEN_USING:
            return parse_using(p, needs_semicolon);
        case TOKEN_SEMICOLON:   // skip random semicolons in the code
            throw_error(p->context, ERR_SYNTAX_WARNING, p->tok, "stray `;` found. If this is intentional, use `noop;`.");
            // fall through
        case TOKEN_NOOP:
            {
                ASTNode_T* noop = init_ast_node(ND_NOOP, p->tok);
                
                if(tok_is(p, TOKEN_NOOP))
                {
                    parser_advance(p);
                    if(needs_semicolon)
                        parser_consume(p, TOKEN_SEMICOLON, "expect `;` after `noop` statement");
                } 
                else 
                {
                    parser_advance(p);
                }
                return noop;
            }
        case TOKEN_EXTERN:
            return parse_extern_c_block(p, needs_semicolon);
        default:
            return parse_expr_stmt(p, needs_semicolon);
    }

    // satisfy -Wall
    return NULL;
}

/////////////////////////////////
// Expression PRATT parser     //
/////////////////////////////////

static ASTNode_T* parse_expr(Parser_T* p, Precedence_T prec, TokenType_T end_tok)
{
    PrefixParseFn_T prefix = get_PrefixParseFn_T(p, p->tok);

    if(!prefix)
        throw_error(p->context, ERR_SYNTAX_ERROR, p->tok, "unexpected token `%s`, expect expression", p->tok->value);

    ASTNode_T* left_expr = prefix(p);

    while(!tok_is(p, end_tok) && prec < get_precedence(p, p->tok))
    {
        InfixParseFn_T infix = get_InfixParseFn_T(p, p->tok);
        if(!infix)
            return left_expr;
        
        left_expr = infix(p, left_expr);
    }

    return left_expr;
}

static List_T* parse_expr_list(Parser_T* p, TokenType_T end_tok, bool allow_unpacking_operators)
{
    List_T* list = init_list();
    mem_add_list(list);

    while (!tok_is(p, end_tok) && !tok_is(p, TOKEN_EOF)) 
    {
        UnpackMode_T umode = UMODE_NONE;
        if(allow_unpacking_operators && tok_is(p, TOKEN_VA_LIST)) {
            parser_advance(p);
            umode = UMODE_FTOB;
        }

        ASTNode_T* arg = parse_expr(p, PREC_LOWEST, TOKEN_COMMA);

        if(allow_unpacking_operators && tok_is(p, TOKEN_VA_LIST)) {
            if(umode)
                throw_error(p->context, ERR_SYNTAX_ERROR_UNCR, p->tok, "already unpacking front-to-back, cannot unpack twice");
            parser_advance(p);
            umode = UMODE_BTOF;
        }
        arg->unpack_mode = umode;

        list_push(list, arg);

        if(!tok_is(p, end_tok))
            parser_consume(p, TOKEN_COMMA, "expect `,` between call arguments");
    }

    return list;
}

static ASTNode_T* parse_id(Parser_T* p)
{
    ASTNode_T* id = init_ast_node(ND_ID, p->tok);
    id->id = parse_identifier(p);

    if(tok_is_operator(p, STATIC_MEMBER) && parser_peek(p, 1)->type == TOKEN_LBRACE)
        return parse_struct_lit(p, id);

    return id;
}

static ASTNode_T* parse_int_lit(Parser_T* p)
{
    ASTNode_T* lit = init_ast_node(ND_INT, p->tok);
    parser_consume(p, TOKEN_INT, "expect integer literal (0, 1, 2, ...)");
    i64 num = atoll(lit->tok->value);
    if(num <= INT_MAX)
    {
        lit->kind = ND_INT;
        lit->int_val = (i32) num;
        lit->data_type = get_primitive_type("i32");
    }
    else if(num <= LONG_MAX)
    {
        lit->kind = ND_LONG;
        lit->long_val = (i64) num;
        lit->data_type = get_primitive_type("i64");
    }
    else
    {
        lit->kind = ND_ULONG;
        lit->ulong_val = num;
        lit->data_type = get_primitive_type("u64");
    }

    lit->is_constant = true;
    return lit;
}

static ASTNode_T* parse_float_lit(Parser_T* p)
{
    ASTNode_T* lit = init_ast_node(ND_FLOAT, p->tok);
    parser_consume(p, TOKEN_FLOAT, "expect float literal (0, 1, 2.3, ...)");
    f64 num; 
    sscanf(lit->tok->value, "%lf", &num); 

    if(num <= FLT_MAX)
    {
        lit->kind = ND_FLOAT;
        lit->float_val = (float) num;
        lit->data_type = get_primitive_type("f32");
    }
    else
    {
        lit->kind = ND_DOUBLE;
        lit->double_val = num;
        lit->data_type = get_primitive_type("f64");
    }

    return lit;
}

static ASTNode_T* parse_bool_lit(Parser_T* p)
{
    ASTNode_T* bool_lit = constant_literals[p->tok->type];

    bool_lit->bool_val = p->tok->type == TOKEN_TRUE;

    parser_advance(p);

    if(!bool_lit->data_type)
        bool_lit->data_type = (ASTType_T*) primitives[TY_BOOL];

    return bool_lit;
}

static ASTNode_T* parse_nil_lit(Parser_T* p)
{
    ASTNode_T* nil_lit = constant_literals[p->tok->type];
    parser_advance(p);

    if(!nil_lit->data_type)
        nil_lit->data_type = (ASTType_T*) void_ptr_type;

    return nil_lit;
}

static ASTNode_T* parse_char_lit(Parser_T* p)
{
    ASTNode_T* char_lit = init_ast_node(ND_CHAR, p->tok);

    if(strlen(p->tok->value) > 1)
        char_lit->str_val = strdup((char[]){'\\', p->tok->value[1], '\0'});
    else 
        char_lit->str_val = strdup((char[]){p->tok->value[0], '\0'});
    
    char_lit->is_constant = true; 
    char_lit->data_type = (ASTType_T*) primitives[TY_CHAR];

    parser_consume(p, TOKEN_CHAR, "expect char literal ('a', 'b', ...)");
    
    return char_lit;
}

static ASTNode_T* parse_str_lit(Parser_T* p)
{
    ASTNode_T* node = init_ast_node(ND_STR, p->tok);
    node->str_val =  strdup(p->tok->value);
    node->data_type = (ASTType_T*) char_ptr_type;

    parser_consume(p, TOKEN_STRING, "expect string literal (\"abc\", \"wxyz\", ...)");

    while(tok_is(p, TOKEN_STRING)) // expressions like `"h" "e" "l" "l" "o"` get grouped together to `"hello"`
    {
        node->str_val = realloc(node->str_val, (strlen(node->str_val) + strlen(p->tok->value) + 1) * sizeof(char));
        strcat(node->str_val, p->tok->value);
        parser_advance(p);
    }
    
    return node;
}

static ASTNode_T* parse_array_lit(Parser_T* p)
{
    ASTNode_T* a_lit = init_ast_node(ND_ARRAY, p->tok);
    parser_consume(p, TOKEN_LBRACKET, "expect `[` for array literal");
    a_lit->args = parse_expr_list(p, TOKEN_RBRACKET, p->cur_obj);
    parser_consume(p, TOKEN_RBRACKET, "expect `]` after array literal");

    return a_lit;
}

static ASTNode_T* parse_struct_lit(Parser_T* p, ASTNode_T* id)
{
    parser_consume_operator(p, STATIC_MEMBER, "expect `::` before `{`");
    ASTNode_T* struct_lit = init_ast_node(ND_STRUCT, p->tok);
    parser_consume(p, TOKEN_LBRACE, "expect `{` for struct literal");
    struct_lit->args = parse_expr_list(p, TOKEN_RBRACE, false);
    parser_consume(p, TOKEN_RBRACE, "expect `}` after struct literal");

    struct_lit->data_type = init_ast_type(TY_UNDEF, id->tok);
    struct_lit->data_type->id = id->id;

    return struct_lit;
}

static ASTNode_T* parse_anonymous_struct_lit(Parser_T* p)
{
    ASTNode_T* struct_lit = init_ast_node(ND_STRUCT, p->tok);
    parser_consume(p, TOKEN_LBRACE, "expect `{` for struct literal");
    struct_lit->args = parse_expr_list(p, TOKEN_RBRACE, false);
    parser_consume(p, TOKEN_RBRACE, "expect `}` after struct literal");

    return struct_lit;
}

static ASTNode_T* parse_lambda_lit(Parser_T* p)
{
    ASTNode_T* lambda = init_ast_node(ND_LAMBDA, p->tok);
    lambda->args = init_list();
    lambda->data_type = init_ast_type(TY_FN, p->tok);
    lambda->data_type->arg_types = init_list();
    
    mem_add_list(lambda->args);
    mem_add_list(lambda->data_type->arg_types);

    if(tok_is_operator(p, "||"))
        parser_advance(p);
    else 
    {
        parser_consume_operator(p, "|", "expect `|` for lambda function");

        while(!tok_is_operator(p, "|"))
        {
            ASTObj_T* arg = init_ast_obj(OBJ_FN_ARG, p->tok);
            arg->id = parse_simple_identifier(p);
        
            parser_consume_operator(p, ":", "expect `:` between argument name and data type");
            arg->data_type = parse_type(p);

            list_push(lambda->args, arg);
            list_push(lambda->data_type->arg_types, arg->data_type);
            if(!tok_is_operator(p, "|"))
                parser_consume(p, TOKEN_COMMA, "expect `,` between arguments");
        }

        parser_consume_operator(p, "|", "expect `|` after lambda arguments");
    }

    lambda->data_type->base = (ASTType_T*) primitives[TY_VOID];
    if(!tok_is(p, TOKEN_ARROW) && !tok_is_operator(p, "=")) 
        lambda->data_type->base = parse_type(p);
    if(tok_is_operator(p, "="))
    {
        lambda->body = init_ast_node(ND_RETURN, p->tok);
        parser_advance(p);

        lambda->body->return_val = parse_expr(p, PREC_LOWEST, TOKEN_SEMICOLON);
    }
    else
    {
        parser_consume(p, TOKEN_ARROW, "expect `=>` after definition");
        lambda->body = parse_stmt(p, false);
    }

    return lambda;
}

// constant lambda expressions get converted to regular functions
static ASTNode_T* parse_const_lambda(Parser_T* p)
{
    static u64 count = 0;

    ASTObj_T* lambda_fn = init_ast_obj(OBJ_FUNCTION, p->tok);
    lambda_fn->args = init_list();
    lambda_fn->data_type = init_ast_type(TY_FN, p->tok);
    lambda_fn->data_type->arg_types = init_list();
    lambda_fn->data_type->is_constant = true;
    lambda_fn->objs = init_list();

    mem_add_list(lambda_fn->args);
    mem_add_list(lambda_fn->data_type->arg_types);
    mem_add_list(lambda_fn->objs);

    char* id;

    if(p->context->ct == CT_ASM)
    {
        id = calloc(34, sizeof(char));
        sprintf(id, "const.lambda.%ld", count++);
    }
    else
    {
        id = calloc(42, sizeof(char));
        sprintf(id, "__csp_const_lambda_%ld__", count++);
    }

    mem_add_ptr(id);
    lambda_fn->id = init_ast_identifier(p->tok, id);

    if(tok_is_operator(p, "||"))
        parser_advance(p);
    else
    {
        parser_consume_operator(p, "|", "expect `|` for lambda function");

        while(!tok_is_operator(p, "|"))
        {
            ASTObj_T* arg = init_ast_obj(OBJ_FN_ARG, p->tok);
            arg->id = parse_simple_identifier(p);
        
            parser_consume_operator(p, ":", "expect `:` between argument name and data type");
            arg->data_type = parse_type(p);

            list_push(lambda_fn->args, arg);
            list_push(lambda_fn->data_type->arg_types, arg->data_type);
            if(!tok_is_operator(p, "|"))
                parser_consume(p, TOKEN_COMMA, "expect `,` between arguments");
        }

        parser_consume_operator(p, "|", "expect `|` after lambda_fn arguments");
    }

    lambda_fn->data_type->base = lambda_fn->return_type = (ASTType_T*) primitives[TY_VOID];
    if(!tok_is(p, TOKEN_ARROW) && !tok_is_operator(p, "=")) 
        lambda_fn->data_type->base = lambda_fn->return_type = parse_type(p);

    if(tok_is_operator(p, "="))
    {
        lambda_fn->body = init_ast_node(ND_RETURN, p->tok);
        parser_advance(p);

        lambda_fn->body->return_val = parse_expr(p, PREC_LOWEST, TOKEN_SEMICOLON);
    }
    else
    {
        parser_consume(p, TOKEN_ARROW, "expect `=>` after definition");
        lambda_fn->body = parse_stmt(p, false);
    }

    collect_locals(lambda_fn->body, lambda_fn->objs);
    if(p->context->ct == CT_ASM)
        lambda_fn->alloca_bottom = &alloca_bottom;
    list_push(p->root_ref->objs, lambda_fn);

    ASTNode_T* lambda_id = init_ast_node(ND_ID, lambda_fn->tok);
    lambda_id->data_type = lambda_fn->data_type;
    lambda_id->referenced_obj = lambda_fn;
    lambda_id->id = lambda_fn->id;

    return lambda_id;
}

static ASTNode_T* parse_ternary(Parser_T* p)
{
    ASTNode_T* ternary = init_ast_node(ND_TERNARY, p->tok);
    parser_consume(p, TOKEN_IF, "expect `if` keyword");

    ternary->condition = parse_expr(p, PREC_LOWEST, TOKEN_ARROW);
    parser_consume(p, TOKEN_ARROW, "expect `=>` after condition");

    ternary->if_branch = parse_expr(p, PREC_LOWEST, TOKEN_ELSE);
    parser_consume(p, TOKEN_ELSE, "expect `else` between if branches");

    ternary->else_branch = parse_expr(p, PREC_LOWEST, TOKEN_SEMICOLON);

    return ternary;
}

static ASTNode_T* parse_else_expr(Parser_T* p, ASTNode_T* left)
{
    ASTNode_T* else_expr = init_ast_node(ND_ELSE_EXPR, p->tok);
    parser_consume(p, TOKEN_ELSE, "expect `else`");

    else_expr->left = left;
    else_expr->right = parse_expr(p, PREC_INFIX_CALL, TOKEN_SEMICOLON);

    return else_expr;
}

static ASTNode_T* parse_unary(Parser_T* p)
{
    if(strlen(p->tok->value) != 1)
    {
        // TODO: implement custom unary operators
        throw_error(p->context, ERR_SYNTAX_ERROR, p->tok, "custom unary ops not implemented yet.");
    }

    ASTNode_T* unary = init_ast_node(unary_ops[(int) *p->tok->value], p->tok);
    parser_advance(p);

    unary->right = parse_expr(p, PREC_UNARY, TOKEN_EOF);
    return unary;
}

static ASTNode_T* parse_num_op(Parser_T* p, ASTNode_T* left)
{
    ASTNode_T* infix = init_ast_node(get_infix_op(p->tok->value), p->tok);
    parser_advance(p);

    infix->left = left;
    infix->right = parse_expr(p, get_precedence(p, infix->tok), TOKEN_EOF);

    return infix;
}

static ASTNode_T* parse_bit_op(Parser_T* p, ASTNode_T* left)
{
    return parse_num_op(p, left);
}

static ASTNode_T* parse_bool_op(Parser_T* p, ASTNode_T* left)
{
    ASTNode_T* infix = init_ast_node(get_infix_op(p->tok->value), p->tok);
    parser_advance(p);

    infix->left = left;
    infix->right = parse_expr(p, get_precedence(p, infix->tok), TOKEN_EOF);

    infix->data_type = (ASTType_T*) primitives[TY_BOOL]; // set the data type, since == != > >= < <= will always result in booleans

    return infix;
}

static ASTNode_T* parse_assignment(Parser_T* p, ASTNode_T* left)
{
    ASTNode_T* assign = init_ast_node(ND_ASSIGN, p->tok);
    assign->left = left;
    
    Token_T* op = p->tok;
    parser_advance(p);

    ASTNode_T* right = parse_expr(p, PREC_LOWEST, TOKEN_EOF);
    if(is_operator(op, "="))
    {
        assign->right = right;
        assign->right->is_assigning = assign->right->kind == ND_ARRAY || assign->right->kind == ND_STRUCT;
    }
    else
    {
        Token_T* assign_op_tok = duplicate_token(op);
        assign_op_tok->value[strlen(op->value) - 1] = '\0'; // cut the `=` from the operator
        ASTNode_T* assign_op = init_ast_node(get_infix_op(assign_op_tok->value), assign_op_tok);
        assign_op->left = left;
        assign_op->right = right;

        assign->right = assign_op;
    }

    return assign;
}

static ASTNode_T* parse_postfix(Parser_T* p, ASTNode_T* left)
{
    ASTNode_T* postfix = init_ast_node(get_infix_op(p->tok->value), p->tok);
    postfix->left = left;

    parser_advance(p);

    return postfix;
}

static ASTNode_T* parse_call(Parser_T* p, ASTNode_T* left)
{
    ASTNode_T* call = init_ast_node(ND_CALL, p->tok);

    call->expr = left;  // the expression to call

    parser_consume(p, TOKEN_LPAREN, "expect `(` after callee");

    call->args = parse_expr_list(p, TOKEN_RPAREN, true);
    parser_consume(p, TOKEN_RPAREN, "expect `)` after call arguments");

    return call;
}

static ASTNode_T* parse_custom_prefix_operator(Parser_T* p)
{
    ASTNode_T* call = init_ast_node(ND_CALL, p->tok);
    call->expr = init_ast_node(ND_ID, p->tok);
    call->expr->id = init_ast_identifier(p->tok, p->tok->value);

    parser_consume(p, TOKEN_OPERATOR, "expect operator");

    ASTNode_T* arg = parse_expr(p, PREC_UNARY, TOKEN_SEMICOLON);

    call->args = init_list();
    list_push(call->args, arg);

    return call;
}

static ASTNode_T* parse_custom_infix_operator(Parser_T* p, ASTNode_T* left)
{
    ASTNode_T* call = init_ast_node(ND_CALL, p->tok);
    call->expr = init_ast_node(ND_ID, p->tok);
    call->expr->id = init_ast_identifier(p->tok, p->tok->value);

    parser_consume(p, TOKEN_OPERATOR, "expect operator");

    ASTNode_T* right = parse_expr(p, PREC_CUSTOM_OPERATOR, TOKEN_SEMICOLON);

    call->args = init_list();
    list_push(call->args, left);
    list_push(call->args, right);

    return call;
}

static ASTNode_T* parse_index(Parser_T* p, ASTNode_T* left)
{
    ASTNode_T* index = init_ast_node(ND_INDEX, p->tok);
    index->left = left;

    parser_consume(p, TOKEN_LBRACKET, "expect `[` after array name for an index expression");

    if(tok_is_operator(p, "^"))
    {
        parser_advance(p);
        index->from_back = true;
    }
    index->expr = parse_expr(p, PREC_LOWEST, TOKEN_RBRACKET);
    parser_consume(p, TOKEN_RBRACKET, "expect `]` after array index");

    return index;
}

static ASTNode_T* parse_pipe(Parser_T* p, ASTNode_T* left)
{
    ASTNode_T* pipe = init_ast_node(ND_PIPE, p->tok);
    pipe->left = left;

    parser_consume_operator(p, "|>", "expect `|>` for pipe expression");

    parser_enable_holes(p);
    pipe->right = parse_expr(p, PREC_PIPE, TOKEN_SEMICOLON);
    parser_disable_holes(p);

    return pipe;
}

static ASTNode_T* parse_hole(Parser_T* p)
{
    Token_T* tok = p->tok;
    if(!parser_holes_enabled(p))
        throw_error(p->context, ERR_SYNTAX_ERROR, tok, "cannot have `$` here, only use `$` in pipe expressions");
    parser_consume(p, TOKEN_DOLLAR, "expect `$`");
    return init_ast_node(ND_HOLE, tok);
}

static ASTNode_T* parse_const_expr(Parser_T* p)
{
    parser_advance(p);

    if(tok_is_operator(p, "|") || tok_is_operator(p, "||"))
        return parse_const_lambda(p);

    throw_error(p->context, ERR_SYNTAX_ERROR, p->tok, "unknown const expression");
    return NULL;
}

static ASTNode_T* parse_builtin_type_exprs(Parser_T* p, ASTNode_T* expr)
{
    if(streq(p->tok->value, "reg_class"))
    {
        expr->cmp_kind = ND_BUILTIN_REG_CLASS;
        expr->data_type = (ASTType_T*) primitives[TY_I32];
    }
    else if(streq(p->tok->value, "is_int"))
    {
        expr->cmp_kind = ND_BUILTIN_IS_INT;
        expr->data_type = (ASTType_T*) primitives[TY_BOOL];
    }
    else if(streq(p->tok->value, "is_uint"))
    {
        expr->cmp_kind = ND_BUILTIN_IS_UINT;
        expr->data_type = (ASTType_T*) primitives[TY_BOOL];
    }
    else if(streq(p->tok->value, "is_float"))
    {
        expr->cmp_kind = ND_BUILTIN_IS_FLOAT;
        expr->data_type = (ASTType_T*) primitives[TY_BOOL];
    }
    else if(streq(p->tok->value, "is_pointer"))
    {
        expr->cmp_kind = ND_BUILTIN_IS_POINTER;
        expr->data_type = (ASTType_T*) primitives[TY_BOOL];
    }
    else if(streq(p->tok->value, "is_array"))
    {
        expr->cmp_kind = ND_BUILTIN_IS_ARRAY;
        expr->data_type = (ASTType_T*) primitives[TY_BOOL];
    }
    else if(streq(p->tok->value, "is_struct"))
    {
        expr->cmp_kind = ND_BUILTIN_IS_STRUCT;
        expr->data_type = (ASTType_T*) primitives[TY_BOOL];
    }
    else if(streq(p->tok->value, "is_union"))
    {
        expr->cmp_kind = ND_BUILTIN_IS_UNION;
        expr->data_type = (ASTType_T*) primitives[TY_BOOL];
    }
    else if(streq(p->tok->value, "to_str"))
    {
        expr->cmp_kind = ND_BUILTIN_TO_STR;
        expr->data_type = (ASTType_T*) char_ptr_type; 
    }
    else
        throw_error(p->context, ERR_UNDEFINED, p->tok, "Undefined builtin type expression `%s`", p->tok->value);


    parser_consume(p, TOKEN_ID, "expect identifier");
    parser_consume(p, TOKEN_LPAREN, "expect `(` after `reg_class`");
    
    expr->r_type = parse_type(p);
    parser_consume(p, TOKEN_RPAREN, "expect `)` after `reg_class`");
    
    return expr;
}

static ASTNode_T* parse_type_expr(Parser_T* p)
{
    ASTNode_T* expr = init_ast_node(ND_TYPE_EXPR, p->tok);
    parser_consume(p, TOKEN_TYPE, "expect `type` keyword");
    parser_consume_operator(p, STATIC_MEMBER, "expect `::` after `type`");

    if(tok_is(p, TOKEN_LPAREN))
    {
        parser_advance(p);

        expr->l_type = parse_type(p);


        if(tok_is_operator(p, "==") || tok_is_operator(p, "!=") || tok_is_operator(p, ">") || 
            tok_is_operator(p, ">=") || tok_is_operator(p, "<") || tok_is_operator(p, "<="))
        {
            expr->cmp_kind = get_infix_op(p->tok->value);
            parser_advance(p);
        }
        else
            throw_error(p->context, ERR_SYNTAX_ERROR, p->tok, "expect one of `==` `!=` `>` `>=` `<` `<=`, got `%s`", p->tok->value);
        
        expr->r_type = parse_type(p);
        expr->data_type = (ASTType_T*) primitives[TY_BOOL];

        parser_consume(p, TOKEN_RPAREN, "expect `)` after type comparison");
    }
    else if(tok_is(p, TOKEN_ID))
        parse_builtin_type_exprs(p, expr);
    return expr;
}

static ASTNode_T* parse_closure(Parser_T* p)
{
    ASTNode_T* closure = init_ast_node(ND_CLOSURE, p->tok);
    parser_consume(p, TOKEN_LPAREN, "expect `(` to begin closure");

    closure->exprs = init_list();
    mem_add_list(closure->exprs);
    do {
        list_push(closure->exprs, parse_expr(p, PREC_LOWEST, TOKEN_RPAREN));
        if(!tok_is(p, TOKEN_RPAREN))
            parser_consume(p, TOKEN_COMMA, "expect `)` or `,` after closure expression");
    } while(!tok_is(p, TOKEN_EOF) && !tok_is(p, TOKEN_RPAREN));

    parser_consume(p, TOKEN_RPAREN, "expect `)` after closure expression");

    return closure;
}

static ASTNode_T* parse_cast(Parser_T* p, ASTNode_T* left)
{   
    ASTNode_T* cast = init_ast_node(ND_CAST, p->tok);
    parser_consume_operator(p, ":", "expect `:` after expression for type cast");
    cast->left = left;
    cast->data_type = parse_type(p);
    cast->is_constant = left->is_constant;

    return cast;
}

static ASTNode_T* parse_sizeof(Parser_T* p)
{
    ASTNode_T* size_of = init_ast_node(ND_SIZEOF, p->tok);
    parser_consume(p, TOKEN_SIZEOF, "expect `sizeof` keyword");

    size_of->the_type = parse_type(p);
    size_of->data_type = (ASTType_T*) primitives[TY_U64];

    return size_of;
}

static ASTNode_T* parse_alignof(Parser_T* p)
{
    ASTNode_T* align_of = init_ast_node(ND_ALIGNOF, p->tok);
    parser_consume(p, TOKEN_ALIGNOF, "expect `alignof` keyword");

    align_of->the_type = parse_type(p);
    align_of->data_type = (ASTType_T*) primitives[TY_U64];

    return align_of;
}

static ASTNode_T* parse_len(Parser_T* p)
{
    ASTNode_T* len = init_ast_node(ND_LEN, p->tok);
    parser_consume(p, TOKEN_LEN, "expect `len` keyword");

    len->expr = parse_expr(p, PREC_LOWEST, TOKEN_SEMICOLON);
    len->data_type = (ASTType_T*) primitives[TY_U64];

    return len;
}

static ASTNode_T* parse_member(Parser_T* p, ASTNode_T* left)
{
    ASTNode_T* member = init_ast_node(ND_MEMBER, p->tok);
    parser_consume_operator(p, ".", "expect `.` for member expression");

    member->left = left;
    member->right = parse_expr(p, PREC_MEMBER, TOKEN_SEMICOLON);

    if(member->right->kind != ND_ID)
        throw_error(p->context, ERR_SYNTAX_ERROR, member->right->tok, "expect identifier");

    return member;
}

static ASTNode_T* parse_infix_call_expr(Parser_T* p)
{
    ASTNode_T* infix_id = init_ast_node(ND_ID, p->tok);

    parser_consume(p, TOKEN_INFIX_CALL, "expect infix call name before infix function call");
    infix_id->id = parse_identifier(p);
    parser_consume(p, TOKEN_INFIX_CALL, "expect infix call name after infix function call");

    return infix_id;
}

static ASTNode_T* parse_infix_call(Parser_T* p, ASTNode_T* left)
{
    ASTNode_T* call = init_ast_node(ND_CALL, p->tok);
    call->expr = parse_infix_call_expr(p);
    call->args = init_list();
    list_push(call->args, left);
    list_push(call->args, parse_expr(p, PREC_INFIX_CALL, TOKEN_SEMICOLON));

    mem_add_list(call->args);
    return call;
}

static ASTNode_T* parse_pow_2(Parser_T* p, ASTNode_T* left)
{
    // x² = (x * x)
    ASTNode_T* mult = init_ast_node(ND_MUL, p->tok);
    parser_consume(p, TOKEN_POW_2, "expect `²`");

    mult->left = left;
    mult->right = left;

    if(p->context->ct == CT_TRANSPILE)
    {
        ASTNode_T* closure = init_ast_node(ND_CLOSURE, p->tok);
        closure->exprs = init_list();
        mem_add_list(closure->exprs); 
        list_push(closure->exprs, mult);
        return closure;
    }

    return mult;
}

static ASTNode_T* parse_pow_3(Parser_T* p, ASTNode_T* left)
{
    // x³ = (x * x * x)
    ASTNode_T* mult_a = init_ast_node(ND_MUL, p->tok);
    ASTNode_T* mult_b = init_ast_node(ND_MUL, p->tok);
    parser_consume(p, TOKEN_POW_3, "expect `³`");

    mult_a->left = left;
    mult_a->right = mult_b;
    mult_b->left = left;
    mult_b->right = left;

    if(p->context->ct == CT_TRANSPILE)
    {
        ASTNode_T* closure = init_ast_node(ND_CLOSURE, p->tok);
        closure->exprs = init_list();
        mem_add_list(closure->exprs); 
        list_push(closure->exprs, mult_a);
        return closure;
    }

    return mult_a;
}

static ASTNode_T* parse_current_fn_token(Parser_T* p)
{
    p->tok->type = TOKEN_STRING;
    p->tok = mem_realloc(p->tok, sizeof(Token_T) + strlen(p->cur_obj->id->callee) + 1);
    strcpy(p->tok->value, p->cur_obj->id->callee);

    return parse_str_lit(p);
}