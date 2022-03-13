#include "optimizer.h"
#include "ast/ast.h"
#include "ast/ast_iterator.h"
#include "config.h"
#include "error/error.h"
#include "io/log.h"
#include "list.h"

#define throw_error(...)              \
    do {                              \
        fprintf(OUTPUT_STREAM, "\n"); \
        throw_error(__VA_ARGS__);     \
    } while(0)

void remove_dead_code(ASTProg_T* ast);

void optimize(ASTProg_T *ast)
{
    static struct {
        void (*fn)(ASTProg_T*);
        const char* description;
    } passes[] = {
        {remove_dead_code, "remove dead code"}
    };

    u32 count = sizeof(passes) / sizeof(*passes);

    for(u32 i = 0; i < count; i++)
    {
        if(!global.silent)
        {
            LOG_OK_F("%s" COLOR_BOLD_GREEN "  Optimizing" COLOR_RESET " (%d/%d): %s", i ? "\33[2K\r" : "", i + 1, count, passes[i].description);
            fflush(OUTPUT_STREAM);
        }
        passes[i].fn(ast);
    }

    if(count)
        fprintf(OUTPUT_STREAM, "\n");
}

void remove_dead_code(ASTProg_T* ast)
{
    ast->entry_point->referenced = true;

    List_T* node_stack = init_list(sizeof(struct AST_NODE_STRUCT*));
    list_push(node_stack, ast->entry_point->body);

    while(node_stack->size)
    {
        ASTNode_T* stack_top = list_pop(node_stack);

        if(!stack_top)
            continue;

        switch(stack_top->kind)
        {
            case ND_ASM:
                for(size_t i = 0; i < stack_top->args->size; i++) 
                {
                    ASTNode_T* arg = stack_top->args->items[i];
                    if(arg->kind == ND_ID)
                        arg->referenced_obj->referenced = true;
                }
                break;

            case ND_ADD:
            case ND_SUB:
            case ND_MUL:
            case ND_DIV:
            case ND_MOD:
            case ND_EQ:
            case ND_NE:
            case ND_GT:
            case ND_GE:
            case ND_LT:
            case ND_LE:
            case ND_AND:
            case ND_OR:
            case ND_LSHIFT:
            case ND_RSHIFT:
            case ND_XOR:
            case ND_BIT_OR:
            case ND_BIT_AND:
            case ND_ASSIGN:
            case ND_MEMBER:
            case ND_PIPE:
                list_push(node_stack, stack_top->left);
                list_push(node_stack, stack_top->right);
                break;

            case ND_CLOSURE:
            case ND_LEN:
            case ND_EXPR_STMT:
                list_push(node_stack, stack_top->expr);
                break;

            case ND_NEG:
            case ND_BIT_NEG:
            case ND_NOT:
            case ND_REF:
            case ND_DEREF:
                list_push(node_stack, stack_top->right);
                break;

            case ND_INDEX:
                list_push(node_stack, stack_top->expr);
            case ND_INC:
            case ND_DEC:
            case ND_CAST:
                list_push(node_stack, stack_top->left);
                break;

            case ND_CALL:
                list_push(node_stack, stack_top->expr);
                for(size_t i = 0; i < stack_top->args->size; i++)
                    list_push(node_stack, stack_top->args->items[i]);
                break;

            case ND_WITH:
                stack_top->exit_fn->referenced = true;
                if(stack_top->exit_fn->kind == OBJ_FUNCTION && !stack_top->exit_fn->is_extern)
                        list_push(node_stack, stack_top->exit_fn->body);
            case ND_IF_EXPR:
            case ND_IF:
                list_push(node_stack, stack_top->condition);
                list_push(node_stack, stack_top->if_branch);
                if(stack_top->else_branch)
                    list_push(node_stack, stack_top->else_branch);
                break;

            case ND_CASE:
            case ND_WHILE:
                list_push(node_stack, stack_top->condition);
            case ND_LOOP:
            case ND_CASE_TYPE:
                list_push(node_stack, stack_top->body);
                break;

            case ND_FOR:
                list_push(node_stack, stack_top->init_stmt);
                list_push(node_stack, stack_top->condition);
                list_push(node_stack, stack_top->expr);
                list_push(node_stack, stack_top->body);
                break;

            case ND_MATCH:
                list_push(node_stack, stack_top->condition);
                for(size_t i = 0; i < stack_top->cases->size; i++)
                    list_push(node_stack, stack_top->cases->items[i]);
                if(stack_top->default_case)
                    list_push(node_stack, stack_top->default_case);
                break;

            case ND_ARRAY:
            case ND_STRUCT:
                for(size_t i = 0; i < stack_top->args->size; i++)
                    list_push(node_stack, stack_top->args->items[i]);
                break;    

            case ND_BLOCK:
                for(size_t i = 0; i < stack_top->stmts->size; i++)
                    list_push(node_stack, stack_top->stmts->items[i]);
                break;

            case ND_RETURN:
                if(stack_top->return_val)
                    list_push(node_stack, stack_top->return_val);
                break;

            case ND_ID:
                if(stack_top->referenced_obj && !stack_top->referenced_obj->referenced)
                {
                    stack_top->referenced_obj->referenced = true;
                    if(stack_top->referenced_obj->kind == OBJ_FUNCTION && !stack_top->referenced_obj->is_extern)
                        list_push(node_stack, stack_top->referenced_obj->body);
                }
                break;

            default:
                break;
        }

    }

    free_list(node_stack);
}