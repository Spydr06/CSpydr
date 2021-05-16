#include "parser.h"
#include "../io/log.h"
#include "optimizer.h"
#include "../io/io.h"

#include <string.h>
#include <stdio.h>

#ifdef __linux__
    #include <libgen.h>
#endif

#include <unistd.h>

/////////////////////////////////
// Expression parsing settings //
/////////////////////////////////

#define NUM_PREFIX_PARSE_FNS 15
#define NUM_INFIX_PARSE_FNS  19
#define NUM_PRECEDENCES      19

static ASTExpr_T* parserParseIdentifier(parser_T* parser);
static ASTExpr_T* parserParseFloat(parser_T* parser);
static ASTExpr_T* parserParseInt(parser_T* parser);
static ASTExpr_T* parserParseBool(parser_T* parser);
static ASTExpr_T* parserParseChar(parser_T* parser);
static ASTExpr_T* parserParseString(parser_T* parser);
static ASTExpr_T* parserParseNot(parser_T* parser);
static ASTExpr_T* parserParseNegate(parser_T* parser);
static ASTExpr_T* parserParseClosure(parser_T* parser);
static ASTExpr_T* parserParseArray(parser_T* parser);
static ASTExpr_T* parserParseDeref(parser_T* parser);
static ASTExpr_T* parserParseRef(parser_T* parser);
static ASTExpr_T* parserParseNil(parser_T* parser);
static ASTExpr_T* parserParseStruct(parser_T* parser);

struct {tokenType_T tt; prefixParseFn fn;} prefixParseFns[NUM_PREFIX_PARSE_FNS] = {
    {TOKEN_ID, parserParseIdentifier},
    {TOKEN_INT, parserParseInt},
    {TOKEN_FLOAT, parserParseFloat},
    {TOKEN_NIL, parserParseNil},
    {TOKEN_TRUE, parserParseBool},
    {TOKEN_FALSE, parserParseBool},
    {TOKEN_CHAR, parserParseChar},
    {TOKEN_STRING, parserParseString},
    {TOKEN_BANG, parserParseNot},
    {TOKEN_MINUS, parserParseNegate},
    {TOKEN_LPAREN, parserParseClosure},
    {TOKEN_LBRACKET, parserParseArray},
    {TOKEN_LBRACE, parserParseStruct},
    {TOKEN_STAR, parserParseDeref},
    {TOKEN_REF, parserParseRef},
};

static ASTExpr_T* parserParseInfixExpression(parser_T* parser, ASTExpr_T* left);
static ASTExpr_T* parserParseCallExpression(parser_T* parser, ASTExpr_T* left);
static ASTExpr_T* parserParseIndexExpression(parser_T* parser, ASTExpr_T* left);
static ASTExpr_T* parserParsePostfixExpression(parser_T* parser, ASTExpr_T* left);
static ASTExpr_T* parserParseAssignment(parser_T* parser, ASTExpr_T* left);

struct {tokenType_T tt; infixParseFn fn;} infixParseFns[NUM_INFIX_PARSE_FNS] = {
    {TOKEN_PLUS, parserParseInfixExpression},
    {TOKEN_MINUS, parserParseInfixExpression},
    {TOKEN_STAR, parserParseInfixExpression},
    {TOKEN_SLASH, parserParseInfixExpression},
    {TOKEN_EQ, parserParseInfixExpression},
    {TOKEN_NOT_EQ, parserParseInfixExpression},
    {TOKEN_GT, parserParseInfixExpression},
    {TOKEN_LT, parserParseInfixExpression},
    {TOKEN_GT_EQ, parserParseInfixExpression},
    {TOKEN_LT_EQ, parserParseInfixExpression},
    {TOKEN_LPAREN, parserParseCallExpression},
    {TOKEN_LBRACKET, parserParseIndexExpression},
    {TOKEN_INC, parserParsePostfixExpression},
    {TOKEN_DEC, parserParsePostfixExpression},
    {TOKEN_ASSIGN, parserParseAssignment},
    {TOKEN_ADD, parserParseAssignment},
    {TOKEN_MULT, parserParseAssignment},
    {TOKEN_SUB, parserParseAssignment},
    {TOKEN_DIV, parserParseAssignment},
};

struct {tokenType_T tt; precedence_T prec;} precedences[NUM_PRECEDENCES] = {
    {TOKEN_EQ, EQUALS},
    {TOKEN_NOT_EQ, EQUALS},
    {TOKEN_LT, LTGT},
    {TOKEN_GT, LTGT},
    {TOKEN_LT_EQ, LTGT},
    {TOKEN_GT_EQ, LTGT},
    {TOKEN_PLUS, SUM},
    {TOKEN_MINUS, SUM},
    {TOKEN_STAR, PRODUCT},
    {TOKEN_SLASH, PRODUCT},
    {TOKEN_LPAREN, CALL},
    {TOKEN_LBRACKET, INDEX},
    {TOKEN_INC, POSTFIX},
    {TOKEN_DEC, POSTFIX},
    {TOKEN_ASSIGN, ASSIGN},
    {TOKEN_ADD, ASSIGN},
    {TOKEN_SUB, ASSIGN},
    {TOKEN_MULT, ASSIGN},
    {TOKEN_DIV, ASSIGN},
};

/////////////////////////////////
// helperfunctions             //
/////////////////////////////////

parser_T* initParser(lexer_T* lexer)
{
    parser_T* parser = calloc(1, sizeof(struct PARSER_STRUCT));
    parser->lexer = lexer;
    parser->eh = parser->lexer->eh;
    parser->localVars = initList(sizeof(struct AST_LOCAL_STRUCT*));
    parser->tok = lexerNextToken(parser->lexer);
    parser->imports = initList(sizeof(char*));

    return parser;
}

void freeParser(parser_T* parser)
{
    freeList(parser->localVars);
    freeToken(parser->tok);

    for(int i = 0; i < parser->imports->size; i++)
        free((char*) parser->imports->items[i]);
    freeList(parser->imports);

    free(parser);
}

token_T* parserAdvance(parser_T* parser)
{
    freeToken(parser->tok);
    parser->tok = lexerNextToken(parser->lexer);
    return parser->tok;
}

bool tokIs(parser_T* parser, tokenType_T type)
{
    return parser->tok->type == type;
}

bool streq(char* s1, char* s2)
{
    return strcmp(s1, s2) == 0;
}

token_T* parserConsume(parser_T* parser, tokenType_T type, const char* msg)
{
    if(!tokIs(parser, type))
    {
        throwSyntaxError(parser->eh, msg, parser->tok->line, parser->tok->pos);
    }

    return parserAdvance(parser);
}

prefixParseFn getPrefixParseFn(tokenType_T type)
{
    for(int i = 0; i < NUM_PREFIX_PARSE_FNS; i++)
        if(prefixParseFns[i].tt == type)
            return prefixParseFns[i].fn;

    return NULL;
}

infixParseFn getInfixParseFn(tokenType_T type)
{
    for(int i = 0; i < NUM_INFIX_PARSE_FNS; i++)
        if(infixParseFns[i].tt == type)
            return infixParseFns[i].fn;

    return NULL;
}

precedence_T getPrecedence(token_T* token)
{
    for(int i = 0; i < NUM_PRECEDENCES; i++)
        if(precedences[i].tt == token->type)
            return precedences[i].prec;

    return LOWEST;
}

bool expressionIsExecutable(ASTExpr_T* expr)    // checks if the expression can be used as a statement ("executable" means it has to assign something)
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

static ASTFile_T* parserParseFile(parser_T* parser, const char* filePath, ASTProgram_T* programRef);

ASTProgram_T* parserParse(parser_T* parser, const char* mainFile)
{
    ASTProgram_T* program = initASTProgram(mainFile);

    listPush(program->files, parserParseFile(parser, mainFile, program));

    optimizer_T* opt = initOptimizer(parser->eh);
    optimizeAST(opt, program);
    freeOptimizer(opt);

    return program;    
}

static ASTGlobal_T* parserParseGlobal(parser_T* parser);
static ASTFunction_T* parserParseFunction(parser_T* parser);
static void parserParseImport(parser_T* parser, ASTProgram_T* programRef);
static ASTTypedef_T* parserParseTypedef(parser_T* parser);
static ASTCompound_T* parserParseCompound(parser_T* parser);

static ASTFile_T* parserParseFile(parser_T* parser, const char* filePath, ASTProgram_T* programRef)
{
    LOG_OK_F(COLOR_BOLD_GREEN "  Compiling" COLOR_RESET " \"%s\"\n", filePath);
    ASTFile_T* root = initASTFile(parser->lexer->file->path);

    while(!tokIs(parser, TOKEN_EOF))
    {
        switch(parser->tok->type)
        {
            case TOKEN_LET:
                listPush(root->globals, parserParseGlobal(parser));
                break;
            case TOKEN_FN:
                listPush(root->functions, parserParseFunction(parser));
                break;
            case TOKEN_IMPORT:
                parserParseImport(parser, programRef);
                break;
            case TOKEN_TYPE:
                listPush(root->types, parserParseTypedef(parser));
                break;

            default:
                throwSyntaxError(parser->eh, "unexpected token", parser->tok->line, parser->tok->pos);
                break;
        }
    }

    return root;
}

static ASTStructType_T* parserParseStructType(parser_T* parser);
static ASTEnumType_T* parserParseEnumType(parser_T* parser);

static ASTType_T* parserParseType(parser_T* parser)
{
    char* type = strdup(parser->tok->value);
    ASTDataType_T dt = AST_TYPEDEF;
    void* body = NULL;
    ASTType_T* subtype = NULL;

    if(streq(type, "i32")) {
        parserAdvance(parser);
        dt = AST_I32;
    }
    else if(streq(type, "i64")) {
        parserAdvance(parser);
        dt = AST_I64;
    }
    else if(streq(type, "u32")) {
        parserAdvance(parser);
        dt = AST_U32;
    }
    else if(streq(type, "u64")) {
        parserAdvance(parser);
        dt = AST_U64;
    }
    else if(streq(type, "f32")) {
        parserAdvance(parser);
        dt = AST_F32;
    }
    else if(streq(type, "f64")) {
        parserAdvance(parser);
        dt = AST_F64;
    }
    else if(streq(type, "bool")) {
        parserAdvance(parser);
        dt = AST_BOOL;
    }
    else if(streq(type, "char")) {
        parserAdvance(parser);
        dt = AST_CHAR;
    }
    else if(streq(type, "char")) {
        parserAdvance(parser);
        dt = AST_CHAR;
    }
    else if(streq(type, "struct")) 
    {
        dt = AST_STRUCT;
        body = parserParseStructType(parser);
    }
    else if(streq(type, "enum"))
    {
        dt = AST_ENUM;
        body = parserParseEnumType(parser);
    }
    else if(streq(type, "*"))
    {
        parserConsume(parser, TOKEN_STAR, "expect `*` for pointer type");
        dt = AST_POINTER;
        subtype = parserParseType(parser);
    }
    else if(streq(type, "["))
    {
        parserConsume(parser, TOKEN_LBRACKET, "expect `[` for array type");
        parserConsume(parser, TOKEN_RBRACKET, "expect `]` for array type");
        dt = AST_ARRAY;
        subtype = parserParseType(parser);
    }

    if(dt == AST_TYPEDEF)   // no special type was found, skip.
        parserAdvance(parser);

    ASTType_T* t = initASTType(dt, subtype, body, type);
    free(type);
    return t;
}

static ASTStructType_T* parserParseStructType(parser_T* parser)
{
    parserConsume(parser, TOKEN_STRUCT, "expect `struct` keyword");
    parserConsume(parser, TOKEN_LBRACE, "expect `{` after struct");

    list_T* names = initList(sizeof(char*));
    list_T* types = initList(sizeof(ASTType_T*));

    while(!tokIs(parser, TOKEN_RBRACE))
    {
        listPush(names, strdup(parser->tok->value));
        parserConsume(parser, TOKEN_ID, "expect struct field name");
        parserConsume(parser, TOKEN_COLON, "expect `:` after field name");

        listPush(types, parserParseType(parser));
        
        if(tokIs(parser, TOKEN_EOF))
        {
            throwSyntaxError(parser->eh, "unclosed struct body", parser->tok->line, parser->tok->pos);
            exit(1);
        }
        else if(!tokIs(parser, TOKEN_RBRACE))
            parserConsume(parser, TOKEN_COMMA, "expect `,` between struct fields");
    }
    parserAdvance(parser);

    if(names->size != types->size)
        LOG_ERROR_F("struct fields have different size: {names: %ld, types: %ld}", names->size, types->size);

    return initASTStructType(types, names);
}

static ASTEnumType_T* parserParseEnumType(parser_T* parser)
{
    parserConsume(parser, TOKEN_ENUM, "expect `enum` keyword");
    parserConsume(parser, TOKEN_LBRACE, "expect `{` after enum");

    list_T* fields = initList(sizeof(char*));
    
    while(!tokIs(parser, TOKEN_RBRACE))
    {
        listPush(fields, strdup(parser->tok->value));
        parserConsume(parser, TOKEN_ID, "expect enum field name");

        if(tokIs(parser, TOKEN_EOF))
        {
            throwSyntaxError(parser->eh, "unclosed enum body", parser->tok->line, parser->tok->pos);
            exit(1);
        }
        else if(!tokIs(parser, TOKEN_RBRACE))
            parserConsume(parser, TOKEN_COMMA, "expect `,` between enum fields");
    }
    parserAdvance(parser);
    return initASTEnumType(fields);
}

static ASTExpr_T* parserParseExpr(parser_T* parser, precedence_T precedence);

static list_T* parserParseExpressionList(parser_T* parser, tokenType_T end)
{
    list_T* exprs = initList(sizeof(struct AST_EXPRESSION_STRUCT*));

    unsigned int startLine = parser->tok->line, startPos = parser->tok->pos;

    while(!tokIs(parser, end))
    {
        listPush(exprs, parserParseExpr(parser, LOWEST));
        
        if(!tokIs(parser, end))
            parserConsume(parser, TOKEN_COMMA, "expect `,` between expressions");
        else if(tokIs(parser, TOKEN_EOF))
        {
            throwSyntaxError(parser->eh, "unclosed expession list", startLine, startPos);
            exit(1);
        }
    }

    return exprs;
}

/////////////////////////////////
// Expressin PRATT parser      //
/////////////////////////////////

static ASTExpr_T* parserParseExpr(parser_T* parser, precedence_T precedence)
{
    prefixParseFn prefix = getPrefixParseFn(parser->tok->type);
    if(prefix == NULL)
    {
        const char* template = "no prefix parse functio for `%s` found";
        char* msg = calloc(strlen(template) + strlen(parser->tok->value) + 1, sizeof(char));
        sprintf(msg, template, parser->tok->value);

        throwSyntaxError(parser->eh, msg, parser->tok->line, parser->tok->pos);
        free(msg);
        exit(1);
    }
    ASTExpr_T* leftExp = prefix(parser);

    while(!tokIs(parser, TOKEN_SEMICOLON) && precedence < getPrecedence(parser->tok))
    {
        infixParseFn infix = getInfixParseFn(parser->tok->type);
        if(infix == NULL)
            return leftExp;
        
        leftExp = infix(parser, leftExp);
    }

    return leftExp;
}

static ASTExpr_T* parserParseIdentifier(parser_T* parser) 
{
    ASTIdentifier_T* id = initASTIdentifier(parser->tok->value, NULL);
    parserConsume(parser, TOKEN_ID, "expect identifier");

    if(tokIs(parser, TOKEN_DOT))
    {
        parserAdvance(parser);
        ASTExpr_T* expr = parserParseIdentifier(parser);
        id->childId = ((ASTIdentifier_T*) expr->expr);
        free(expr);
    }

    return initASTExpr(NULL, EXPR_IDENTIFIER, id);
}

static ASTExpr_T* parserParseInt(parser_T* parser)
{
    ASTExpr_T* ast = initASTExpr(initASTType(AST_I32, NULL, NULL, ""), EXPR_INT_LITERAL, initASTInt(atoi(parser->tok->value)));
    parserConsume(parser, TOKEN_INT, "expect number");
    return ast;
}

static ASTExpr_T* parserParseFloat(parser_T* parser)
{
    ASTExpr_T* ast = initASTExpr(initASTType(AST_F32, NULL, NULL, ""), EXPR_FLOAT_LITERAL, initASTFloat(atof(parser->tok->value)));
    parserConsume(parser, TOKEN_FLOAT, "expect number");
    return ast;
}

static ASTExpr_T* parserParseBool(parser_T* parser)
{
    bool boolVal;
    if(tokIs(parser, TOKEN_TRUE))
        boolVal = true;
    else if(tokIs(parser, TOKEN_FALSE))
        boolVal = false;
    else {
        throwSyntaxError(parser->eh, "not a bool value", parser->tok->line, parser->tok->pos);
        exit(1);
    }

    ASTExpr_T* ast = initASTExpr(initASTType(AST_BOOL, NULL, NULL, ""), EXPR_BOOL_LITERAL, initASTBool(boolVal));
    parserAdvance(parser);
    return ast;
}

static ASTExpr_T* parserParseChar(parser_T* parser)
{
    ASTExpr_T* ast = initASTExpr(initASTType(AST_CHAR, NULL, NULL, ""), EXPR_CHAR_LITERAL, initASTChar(parser->tok->value[0]));
    parserConsume(parser, TOKEN_CHAR, "expect character");
    return ast;
}

static ASTExpr_T* parserParseString(parser_T* parser)
{
    ASTExpr_T* ast = initASTExpr(initASTType(AST_POINTER, initASTType(AST_CHAR, NULL, NULL, ""), NULL, ""), EXPR_STRING_LITERAL, initASTString(parser->tok->value));
    parserConsume(parser, TOKEN_STRING, "expect string");
    return ast;
}

static ASTExpr_T* parserParseNil(parser_T* parser)
{
    parserConsume(parser, TOKEN_NIL, "expect `nil`");
    return initASTExpr(initASTType(AST_POINTER, initASTType(AST_VOID, NULL, NULL, ""), NULL, ""), EXPR_NIL, initASTNil());  // nil is just *void 0
}

static ASTExpr_T* parserParseArray(parser_T* parser)
{
    parserConsume(parser, TOKEN_LBRACKET, "expect `[` for array literal");
    list_T* indexes = parserParseExpressionList(parser, TOKEN_RBRACKET);
    parserAdvance(parser);

    return initASTExpr(initASTType(AST_ARRAY, NULL, NULL, ""), EXPR_ARRAY_LITERAL, initASTArray(indexes));
}

static ASTExpr_T* parserParseStruct(parser_T* parser)
{
    parserConsume(parser, TOKEN_LBRACE, "expect `{` for struct literal");
    list_T* fields = initList(sizeof(char*));
    list_T* exprs = initList(sizeof(struct AST_EXPRESSION_STRUCT*));

    unsigned int startLine = parser->tok->line, startPos = parser->tok->pos;

    while(!tokIs(parser, TOKEN_RBRACE))
    {
        listPush(fields, strdup(parser->tok->value));
        parserConsume(parser, TOKEN_ID, "expect struct field name");
        parserConsume(parser, TOKEN_COLON, "expect `:` after struct field");
        listPush(exprs, parserParseExpr(parser, LOWEST));

        if(tokIs(parser, TOKEN_EOF))
        {
            throwSyntaxError(parser->eh, "unclosed struct literal body, expect `}`", startLine, startPos);
            exit(1);
        }
        else if(!tokIs(parser, TOKEN_RBRACE))
            parserConsume(parser, TOKEN_COMMA, "expect `,` between struct fields");
    }
    parserAdvance(parser);

    return initASTExpr(initASTType(AST_STRUCT, NULL, initASTStructType(initList(sizeof(struct AST_TYPE_STRUCT*)), initList(sizeof(char*))), ""), EXPR_STRUCT_LITERAL, initASTStruct(exprs, fields));
}

static ASTExpr_T* parserParseInfixExpression(parser_T* parser, ASTExpr_T* left)
{
    precedence_T prec = getPrecedence(parser->tok);
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
            throwSyntaxError(parser->eh, "undefined infix expression", parser->tok->line, parser->tok->pos);
            exit(1);
            break;
    }
    parserAdvance(parser);
    ASTExpr_T* right = parserParseExpr(parser, prec);

    return initASTExpr(NULL, EXPR_INFIX, initASTInfix(op, right, left));
}

static ASTExpr_T* parserParseCallExpression(parser_T* parser, ASTExpr_T* left)
{
    if(left->type != EXPR_IDENTIFIER)
    {
        throwSyntaxError(parser->eh, "expect method name for call", parser->tok->line, parser->tok->pos);
        exit(1);
    }

    parserConsume(parser, TOKEN_LPAREN, "epxect `(` for function call");
    list_T* args = parserParseExpressionList(parser, TOKEN_RPAREN);
    parserConsume(parser, TOKEN_RPAREN, "expect `)` after function call arguments");    

    ASTExpr_T* ast = initASTExpr(NULL, EXPR_CALL, initASTCall(((ASTIdentifier_T*) left->expr)->callee, args));
    freeASTExpr(left);  // free the left ast node because we only store the callee
    return ast;
}

static ASTExpr_T* parserParseIndexExpression(parser_T* parser, ASTExpr_T* left)
{
    parserConsume(parser, TOKEN_LBRACKET, "epxect `[` for index expression");
    ASTExpr_T* index = parserParseExpr(parser, LOWEST);
    parserConsume(parser, TOKEN_RBRACKET, "expect `]` after array index");

    return initASTExpr(NULL, EXPR_INDEX, initASTIndex(left, index));
}

static ASTExpr_T* parserParseNot(parser_T* parser)
{
    parserConsume(parser, TOKEN_BANG, "expect `!` for `not` operator");
    return initASTExpr(initASTType(AST_BOOL, NULL, NULL, ""), EXPR_PREFIX, initASTPrefix(OP_NOT, parserParseExpr(parser, LOWEST)));
}

static ASTExpr_T* parserParseNegate(parser_T* parser)
{
    parserConsume(parser, TOKEN_MINUS, "expect `-` for `negate` operator");
    return initASTExpr(NULL, EXPR_PREFIX, initASTPrefix(OP_NEGATE, parserParseExpr(parser, LOWEST)));
}

static ASTExpr_T* parserParseDeref(parser_T* parser)
{
    parserConsume(parser, TOKEN_STAR, "expect `*` to dereference a pointer");
    return initASTExpr(NULL, EXPR_PREFIX, initASTPrefix(OP_DEREF, parserParseExpr(parser, LOWEST)));
}

static ASTExpr_T* parserParseRef(parser_T* parser)
{
    parserConsume(parser, TOKEN_REF, "expect `&` to get a pointer");
    return initASTExpr(NULL, EXPR_PREFIX, initASTPrefix(OP_REF, parserParseExpr(parser, LOWEST)));
}

static ASTExpr_T* parserParseClosure(parser_T* parser)
{
    parserConsume(parser, TOKEN_LPAREN, "expect `(` for closure");
    ASTExpr_T* ast = parserParseExpr(parser, LOWEST);
    parserConsume(parser, TOKEN_RPAREN, "expect `)` after closure");
    return ast;
}

static ASTExpr_T* parserParsePostfixExpression(parser_T* parser, ASTExpr_T* left)
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
            throwSyntaxError(parser->eh, "expect `++` or `--`", parser->tok->line, parser->tok->pos);
            exit(1);
    }
    parserAdvance(parser);

    return initASTExpr(NULL, EXPR_POSTFIX, initASTPostfix(op, left));
}

static ASTExpr_T* parserParseAssignmentOp(ASTExpr_T* left, ASTInfixOpType_T op, ASTExpr_T* right)
{
    return initASTExpr(NULL, EXPR_INFIX, 
                initASTInfix(OP_ASSIGN, initASTExpr(
                    NULL, EXPR_INFIX, 
                    initASTInfix(op, right, left)), 
                    initASTExpr(
                        NULL, EXPR_IDENTIFIER, 
                        initASTIdentifier(((ASTIdentifier_T*) left->expr)->callee, NULL)
                    )
                )
            );
}

static ASTExpr_T* parserParseAssignment(parser_T* parser, ASTExpr_T* left)
{
    if(left->type != EXPR_IDENTIFIER)
    {
        throwSyntaxError(parser->eh, "can only assing a value to a variable", parser->tok->line, parser->tok->pos);
        exit(1);
    }

    tokenType_T op = parser->tok->type;
    parserAdvance(parser);
    
    ASTExpr_T* right = parserParseExpr(parser, ASSIGN);
    switch(op)
    {
        case TOKEN_ASSIGN:
            return initASTExpr(NULL, EXPR_INFIX, initASTInfix(OP_ASSIGN, right, left));
        case TOKEN_ADD:
            return parserParseAssignmentOp(left, OP_ADD, right);
        case TOKEN_SUB:
            return parserParseAssignmentOp(left, OP_SUB, right);
        case TOKEN_MULT:
            return parserParseAssignmentOp(left, OP_MULT, right);
        case TOKEN_DIV:
            return parserParseAssignmentOp(left, OP_DIV, right);
        default:
            throwSyntaxError(parser->eh, "unexpected token, expect assignment", parser->tok->line, parser->tok->pos);
            exit(1);
    }
}

/////////////////////////////////
// Statements                  //
/////////////////////////////////

static ASTLoop_T* parserParseLoop(parser_T* parser) // TODO: loops will for now only support while-like syntax; for and foreach come soon
{
    parserConsume(parser, TOKEN_LOOP, "expect `loop` keyword");

    ASTExpr_T* condition = parserParseExpr(parser, LOWEST);
    ASTCompound_T* body = parserParseCompound(parser);

    return initASTLoop(condition, body);
}

static ASTMatch_T* parserParseMatch(parser_T* parser)
{
    parserConsume(parser, TOKEN_MATCH, "expect `match` keyword");
    
    ASTExpr_T* condition = parserParseExpr(parser, LOWEST);
    parserConsume(parser, TOKEN_LBRACE, "expect `{` after match condtion");

    list_T* cases = initList(sizeof(struct AST_EXPRESSION_STRUCT*));
    list_T* bodys = initList(sizeof(struct AST_COMPOUND_STRUCT*));
    ASTCompound_T* defaultBody = NULL;
    while(!tokIs(parser, TOKEN_RBRACE))
    {   
        switch(parser->tok->type)
        {
            case TOKEN_UNDERSCORE:
                if(defaultBody == NULL) {
                    parserConsume(parser, TOKEN_UNDERSCORE, "expect `_` for default case");
                    parserConsume(parser, TOKEN_ARROW, "expect `=>` after match case");
                    defaultBody = parserParseCompound(parser);
                } else
                {
                    throwRedefinitionError(parser->eh, "redefinition of default match case", parser->tok->line, parser->tok->pos);
                    exit(1);
                }
                break;

            case TOKEN_EOF:
                throwSyntaxError(parser->eh, "expect '}' after match statement", parser->tok->line, parser->tok->pos);
                exit(1);
                break;

            default:
                listPush(cases, parserParseExpr(parser, LOWEST));
                parserConsume(parser, TOKEN_ARROW, "expect `=>` after match case");
                listPush(bodys, parserParseCompound(parser));
                break;
        }
    }
    parserAdvance(parser);

    return initASTMatch(condition, cases, bodys, defaultBody);
}

static ASTIf_T* parserParseIf(parser_T* parser)
{
    parserConsume(parser, TOKEN_IF, "expect `if` keyword");
    ASTExpr_T* condition = parserParseExpr(parser, LOWEST);
    ASTCompound_T* ifBody = parserParseCompound(parser);
    ASTCompound_T* elseBody = NULL;

    if(tokIs(parser, TOKEN_ELSE)) {
        parserAdvance(parser);
        elseBody = parserParseCompound(parser);
    }
    
    return initASTIf(condition, ifBody, elseBody);
}

static ASTReturn_T* parserParseReturn(parser_T* parser)
{
    parserConsume(parser, TOKEN_RETURN, "expect `ret` keyword");
    ASTReturn_T* ast = initASTReturn(parserParseExpr(parser, LOWEST));
    parserConsume(parser, TOKEN_SEMICOLON, "expect `;` after return value");
    return ast;
}

static ASTLocal_T* parserParseLocal(parser_T* parser)
{
    parserConsume(parser, TOKEN_LET, "expect `let` keyword");
    char* name = strdup(parser->tok->value);
    parserConsume(parser, TOKEN_ID, "expect variable name");
    parserConsume(parser, TOKEN_COLON, "expect `:` after variable name");

    ASTType_T* type = parserParseType(parser);

    ASTExpr_T* value = NULL;
    if(tokIs(parser, TOKEN_ASSIGN)) {
        parserAdvance(parser); 
        value = parserParseExpr(parser, LOWEST);
    }
    
    parserConsume(parser, TOKEN_SEMICOLON, "expect `;` after variable definition");

    ASTLocal_T* ast = initASTLocal(type, value, name);
    free(name);
    return ast;
}

static ASTExprStmt_T* parserParseExpressionStatement(parser_T* parser)
{
    unsigned int startLine = parser->tok->line, startPos = parser->tok->pos + 1;

    ASTExprStmt_T* ast = initASTExprStmt(parserParseExpr(parser, LOWEST));

    if(!expressionIsExecutable(ast->expr))
        throwSyntaxError(parser->eh, "can only treat assigning expressions as statements (e.g. =, +=, ++)", startLine, startPos);

    parserConsume(parser, TOKEN_SEMICOLON, "expect `;` after expression");
    return ast;
}

static ASTStmt_T* parserParseStatement(parser_T* parser)
{
    void* stmt;
    ASTStmtType_T type;

    switch(parser->tok->type)
    {
        case TOKEN_LOOP:
            stmt = parserParseLoop(parser);
            type = STMT_LOOP;
            break;
        
        case TOKEN_MATCH:
            stmt = parserParseMatch(parser);
            type = STMT_MATCH;
            break;

        case TOKEN_IF:
            stmt = parserParseIf(parser);
            type = STMT_IF;
            break;

        case TOKEN_RETURN:
            type = STMT_RETURN;
            stmt = parserParseReturn(parser);
            break;

        case TOKEN_LET:
            type = STMT_LET;
            stmt = parserParseLocal(parser);
            break;

        case TOKEN_ID:
            type = STMT_EXPRESSION;
            stmt = parserParseExpressionStatement(parser);
            break;
        
        default:
            throwSyntaxError(parser->eh, "expect statement", parser->tok->line, parser->tok->pos);
            exit(1);
            break;
    }

    return initASTStmt(type, stmt);
}

/////////////////////////////////
// base structures             //
/////////////////////////////////

static ASTCompound_T* parserParseCompound(parser_T* parser)
{
    list_T* stmts = initList(sizeof(struct AST_STATEMENT_STRUCT*));

    if(tokIs(parser, TOKEN_LBRACE))
    {
        parserAdvance(parser);
        
        while(!tokIs(parser, TOKEN_RBRACE))
        {
            listPush(stmts, parserParseStatement(parser));

            if(tokIs(parser, TOKEN_EOF))
            {
                throwSyntaxError(parser->eh, "expect '}' after compound", parser->tok->line, parser->tok->pos);
                exit(1);
            }
        }
        parserAdvance(parser);
    } else  // enables to do single statement compounds without braces
    {
        listPush(stmts, parserParseStatement(parser));
    }

    return initASTCompound(stmts);
}

static ASTGlobal_T* parserParseGlobal(parser_T* parser)
{
    parserConsume(parser, TOKEN_LET, "expect `let` keyword");
    char* name = strdup(parser->tok->value);

    unsigned int line = parser->tok->line;
    unsigned int pos = parser->tok->pos;

    parserConsume(parser, TOKEN_ID, "expect variable name");
    parserConsume(parser, TOKEN_COLON, "expect `:` after variable name");

    ASTType_T* type = parserParseType(parser);

    ASTExpr_T* value = NULL;
    if(tokIs(parser, TOKEN_ASSIGN)) {
        parserAdvance(parser); 
        value = parserParseExpr(parser, LOWEST);
    }
    
    parserConsume(parser, TOKEN_SEMICOLON, "expect `;` after variable definition");

    ASTGlobal_T* ast = initASTGlobal(name, type, value, line, pos);
    free(name);
    return ast;
}

static ASTFunction_T* parserParseFunction(parser_T* parser)
{
    parserConsume(parser, TOKEN_FN, "expect `fn` keyword");
    char* name = strdup(parser->tok->value);

    unsigned int line = parser->tok->line;
    unsigned int pos = parser->tok->pos;

    parserConsume(parser, TOKEN_ID, "expect function name");
    parserConsume(parser, TOKEN_LPAREN, "expect `(` after function name");

    list_T* args = initList(sizeof(struct AST_ARGUMENT_STRUCT*));
    while(!tokIs(parser, TOKEN_RPAREN))
    {
        char* argName = strdup(parser->tok->value);
        parserConsume(parser, TOKEN_ID, "expect argument name");
        parserConsume(parser, TOKEN_COLON, "expect `:` after argument name");
        listPush(args, initASTArgument(argName, parserParseType(parser)));
        free(argName);

        if(!tokIs(parser, TOKEN_RPAREN))
            parserConsume(parser, TOKEN_COMMA, "expect `,` between arguments");
    }
    parserAdvance(parser);
    
    ASTType_T* returnType = NULL;
    if(tokIs(parser, TOKEN_COLON)) {
        parserAdvance(parser);
        returnType = parserParseType(parser);
    }
    else {
        returnType = initASTType(AST_VOID, NULL, NULL, "");
    }

    ASTCompound_T* body = parserParseCompound(parser);

    ASTFunction_T* ast = initASTFunction(name, returnType, body, args, line, pos);
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

static void parserParseImport(parser_T* parser, ASTProgram_T* programRef)
{
    parserConsume(parser, TOKEN_IMPORT, "expect `import` keyword");
    char* relativePath = strdup(parser->tok->value);
    parserConsume(parser, TOKEN_STRING, "expect filepath to import");

    char* directory = getDirectoryFromRelativePath(programRef->mainFile);
    char* importPath = calloc(strlen(directory) + strlen(relativePath) + 2, sizeof(char*));
    sprintf(importPath, "%s/%s", directory, relativePath);
    free(relativePath);
    free(directory);

    if(access(importPath, F_OK) != 0)
    {
        const char* template = "could not open file \"%s\": no such file or directory";
        char* message = calloc(strlen(template) + strlen(importPath) + 1, sizeof(char));
        sprintf(message, template, importPath);
        throwUndefinitionError(parser->eh, message, parser->tok->line, parser->tok->pos);
        free(message);
        exit(1);
    }

    parserConsume(parser, TOKEN_SEMICOLON, "expect `;` after import");

    for(int i = 0; i < parser->imports->size; i++)  // check if the file is already included, when its included, skip it. 
                                                    // No error gets thrown, because 2 files including each other is valid in Spydr
    {
        if(strcmp(parser->imports->items[i], importPath) == 0) {
            free(importPath);
            return;
        }
    }
    listPush(parser->imports, importPath);

    // if the file is not included, compile it to a new ASTFile_T.
    srcFile_T* file = readFile(importPath);
    errorHandler_T* eh = initErrorHandler(file);
    lexer_T* lexer = initLexer(file, eh);
    parser_T* _parser = initParser(lexer);

    ASTFile_T* ast = parserParseFile(_parser, importPath, programRef);
    listPush(programRef->files, ast);

    freeParser(_parser);
    freeLexer(lexer);
    freeErrorHandler(eh);
    freeSrcFile(file);
}

static ASTTypedef_T* parserParseTypedef(parser_T* parser)
{
    parserConsume(parser, TOKEN_TYPE, "expect `type` keyword");
    char* name = strdup(parser->tok->value);

    unsigned int line = parser->tok->line;
    unsigned int pos = parser->tok->pos;

    parserConsume(parser, TOKEN_ID, "expect type name");

    parserConsume(parser, TOKEN_COLON, "expect `:` after typename");

    ASTTypedef_T* ast = initASTTypedef(parserParseType(parser), name, line, pos);
    free(name);

    parserConsume(parser, TOKEN_SEMICOLON, "expect `;` after type defintion");
    return ast;
}