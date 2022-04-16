#include "ast/ast.h"
#include "ast/types.h"
#define PARSER_TESTS {"parsing simple main function", test_parsing_simple_main_func},    \
                     {"parsing complex main function", test_parsing_complex_main_func},   \
                     {"parsing binary operators", test_parsing_binary_operators}

#include "lexer/token.h"
#include "parser/parser.h"

#define PARSER_TEST_FUNC(name, src, code) \
void name(void) {                         \
    File_T* file = get_file(1, src);      \
    TEST_ASSERT(file != NULL);            \
    List_T* files = init_list();          \
    list_push(files, file);               \
    ASTProg_T prog;                       \
    parse(&prog, files, true);            \
    code                                  \
}                                                                

PARSER_TEST_FUNC(test_parsing_simple_main_func, "fn main(): i32 { ret 0; }",
{
    TEST_ASSERT(prog.objs != NULL);
    TEST_ASSERT(prog.objs->size == 1);

    ASTObj_T* fn = prog.objs->items[0];
    TEST_ASSERT(fn != NULL);
    TEST_ASSERT(fn->args != NULL);
    TEST_ASSERT(fn->args->size == 0);
    
    ASTType_T* return_type = fn->return_type;
    TEST_ASSERT(return_type != NULL);
    TEST_ASSERT(return_type->kind == TY_I32);

    TEST_ASSERT(fn->body != NULL);
    TEST_ASSERT(fn->body->kind == ND_BLOCK);
    TEST_ASSERT(fn->body->stmts != NULL);
    TEST_ASSERT(fn->body->stmts->size == 1);
    
    ASTNode_T* return_stmt = fn->body->stmts->items[0];
    TEST_ASSERT(return_stmt != NULL);
    TEST_ASSERT(return_stmt->return_val != NULL);
})

PARSER_TEST_FUNC(test_parsing_complex_main_func, "fn main(argc: i32, argv: &&char): i32 { ret 0; } [ignore_unused(\"main\")]",
{
    TEST_ASSERT(prog.objs != NULL);
    TEST_ASSERT(prog.objs->size == 1);

    ASTObj_T* fn = prog.objs->items[0];
    TEST_ASSERT(fn != NULL);
    TEST_ASSERT(fn->args != NULL);
    TEST_ASSERT(fn->args->size == 2);

    ASTObj_T** fn_args = (ASTObj_T**) fn->args->items;
    TEST_ASSERT(fn_args != NULL);
    
    ASTObj_T* arg1 = fn_args[0];
    TEST_ASSERT(arg1 != NULL);
    TEST_ASSERT(arg1->data_type != NULL);
    TEST_ASSERT(arg1->data_type->kind == TY_I32);

    ASTObj_T* arg2 = fn_args[1];
    TEST_ASSERT(arg2 != NULL);
    TEST_ASSERT(arg2->data_type != NULL);
    TEST_ASSERT(arg2->data_type->kind == TY_PTR);
    TEST_ASSERT(arg2->data_type->base != NULL);
    TEST_ASSERT(arg2->data_type->base->kind == TY_PTR);
    TEST_ASSERT(arg2->data_type->base->base != NULL);
    TEST_ASSERT(arg2->data_type->base->base->kind == TY_CHAR);
})

PARSER_TEST_FUNC(test_parsing_binary_operators, "fn main(): i32 { ret (2 * 10 / 5 - -2) % 2 + 4; }",
{
    TEST_ASSERT(prog.objs != NULL);
    TEST_ASSERT(prog.objs->size == 1);

    ASTObj_T* fn = prog.objs->items[0];
    TEST_ASSERT(fn != NULL);

    TEST_ASSERT(fn->body != NULL);
    TEST_ASSERT(fn->body->kind == ND_BLOCK);
    TEST_ASSERT(fn->body->stmts != NULL);
    TEST_ASSERT(fn->body->stmts->size == 1);
    
    ASTNode_T* return_stmt = fn->body->stmts->items[0];
    TEST_ASSERT(return_stmt != NULL);

    ASTNode_T* return_val = return_stmt->return_val;
    TEST_ASSERT(return_val != NULL);
    TEST_ASSERT(return_val->kind == ND_ADD); // +
    TEST_ASSERT(return_val->right != NULL); 
    TEST_ASSERT(return_val->right->kind == ND_INT); // 4
    TEST_ASSERT(return_val->left != NULL);  
    TEST_ASSERT(return_val->left->kind == ND_MOD); // %
    TEST_ASSERT(return_val->left->right != NULL);
    TEST_ASSERT(return_val->left->right->kind == ND_INT); // 2
    TEST_ASSERT(return_val->left->left != NULL);
    TEST_ASSERT(return_val->left->left->kind == ND_CLOSURE); // ()

    ASTNode_T* closure = return_val->left->left;
    TEST_ASSERT(closure->expr != NULL);
    TEST_ASSERT(closure->expr->kind == ND_SUB); // -
    TEST_ASSERT(closure->expr->right != NULL);
    TEST_ASSERT(closure->expr->right->kind == ND_NEG); // -
    TEST_ASSERT(closure->expr->right->right != NULL);
    TEST_ASSERT(closure->expr->right->right->kind == ND_INT); // 2
    TEST_ASSERT(closure->expr->left != NULL);
    TEST_ASSERT(closure->expr->left->kind == ND_DIV); // /
    TEST_ASSERT(closure->expr->left->left != NULL);
    TEST_ASSERT(closure->expr->left->left->kind == ND_MUL); // *
})