#include "directives.h"
#include "ast/ast.h"
#include "config.h"
#include "error/error.h"
#include "list.h"
#include "parser/parser.h"
#include "platform/pkg_config.h"
#include "platform/platform_bindings.h"
#include "mem/mem.h"
#include "context.h"
#include "io/log.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ANY (-1)
#define EVAL_FN(name) static bool eval_##name(DirectiveData_T* data, Context_T* context, ASTProg_T* ast, ASTObj_T* obj)

typedef struct DIRECTIVE_DATA_STRUCT DirectiveData_T;
typedef bool (*EvalDirectiveFn_T)(DirectiveData_T*, Context_T*, ASTProg_T*, ASTObj_T*);

typedef struct DIRECTIVE_STRUCT {
    const char* name;
    i32 num_args;
    ASTObjKind_T following_obj;
    EvalDirectiveFn_T eval;
} Directive_T;

EVAL_FN(cc);
EVAL_FN(cfg);
EVAL_FN(copy);
EVAL_FN(drop);
EVAL_FN(export);
EVAL_FN(link_dir);
EVAL_FN(link_obj);
EVAL_FN(link);
EVAL_FN(no_return);
EVAL_FN(private);

static const Directive_T DIRECTIVES[] = {
    {
        "cc", 
        ANY,
        0,
        eval_cc,
    },
    {
        "cfg",
        1,
        OBJ_ANY,
        eval_cfg,
    },
    {
        "copy",
        2,
        0,
        eval_copy,
    },
    {
        "drop",
        0,
        OBJ_FUNCTION,
        eval_drop,
    },
    {
        "export", 
        1,
        OBJ_FUNCTION | OBJ_GLOBAL,
        eval_export,
    },
    {
        "link_dir",
        ANY,
        0,
        eval_link_dir,
    },
    {
        "link_obj",
        ANY,
        0,
        eval_link_obj,
    },
    {
        "link",
        ANY,
        0,
        eval_link,
    },
    {
        "no_return",
        0,
        OBJ_FUNCTION,
        eval_no_return,
    },
    {
        "private",
        0,
        OBJ_ANY,
        eval_private
    }
};

static const size_t NUM_DIRECTIVES = sizeof(DIRECTIVES) / sizeof(Directive_T);

typedef struct DIRECTIVE_DATA_STRUCT {
    const Directive_T* directive;
    Token_T* name_token;
    List_T* arguments;
} DirectiveData_T;

DirectiveData_T* directive_data(const Directive_T* directive, Token_T* name_token, List_T* arguments) 
{
    DirectiveData_T* dd = malloc(sizeof(DirectiveData_T));
    dd->directive = directive;
    dd->name_token = name_token;
    dd->arguments = arguments;
    return dd;
}

static DirectiveData_T* parse_directive(Parser_T* p);

void parse_directives(Parser_T* p, List_T* objects)
{
    parser_consume(p, TOKEN_LBRACKET, "expect `[` for compiler directive");
    List_T* directives = init_list();
    const char* objs_following = NULL;

    do {
        DirectiveData_T* data = parse_directive(p);
        list_push(directives, data);

        if(data->directive->following_obj && !objs_following)
            objs_following = data->directive->name;

        if(!tok_is(p, TOKEN_RBRACKET))
            parser_consume(p, TOKEN_COMMA, "expect `,` between compiler directives");
    } while(!tok_is(p, TOKEN_RBRACKET) && !tok_is(p, TOKEN_EOF));

    parser_consume(p, TOKEN_RBRACKET, "expect `]` after compiler directive");

    ASTObj_T* obj = NULL;
    if(!objs_following)
        goto eval;

    size_t old_obj_list_size = objects->size;
    parse_obj(p, objects);
    if(objects->size == old_obj_list_size)
        throw_error(parser_context(p), ERR_UNDEFINED, parser_peek(p, 0), "directive `%s` requires an object [const, fn, type, namespace] after `]`");

    obj = objects->items[objects->size - 1];

    for(size_t i = 0; i < directives->size; i++)
    {
        DirectiveData_T* data = directives->items[i];
        ASTObjKind_T expected = data->directive->following_obj;
        if(expected != 0 && !(expected & obj->kind))
            throw_error(parser_context(p), ERR_MISC, data->name_token, "directive `%s` expects %s after `]`, got %s", data->directive->name, obj_kind_to_str(data->directive->following_obj), obj_kind_to_str(obj->kind));
    }

eval:
    ;
    bool remove_obj = false;
    for(size_t i = 0; i < directives->size; i++)
    {
        DirectiveData_T* data = directives->items[i];
        if(data->directive->eval(data, parser_context(p), parser_ast(p), obj))
            remove_obj = true;
    }

    if(remove_obj && obj)
        list_pop(objects);

    for(size_t i = 0; i < directives->size; i++)
    {
        free_list(((DirectiveData_T*) directives->items[i])->arguments);
        free(directives->items[i]);
    }
    free_list(directives);
}

DirectiveData_T* parse_directive(Parser_T* p)
{
    Token_T* name_tok = parser_peek(p, 0);
    const char* name = name_tok->value;
    parser_consume(p, TOKEN_ID, "expect directive name after `[`");

    const Directive_T* directive = NULL;
    for(size_t i = 0; i < NUM_DIRECTIVES; i++)
        if(strcmp(DIRECTIVES[i].name, name) == 0)
        {
            directive = &DIRECTIVES[i];
            break;
        }

    if(!directive)
        throw_error(parser_context(p), ERR_UNDEFINED, name_tok, "undefined compiler directive `%s`", name);

    List_T* args = init_list();

    if(!tok_is(p, TOKEN_RBRACKET) && !tok_is(p, TOKEN_COMMA))
    {
        parser_consume(p, TOKEN_LPAREN, "expect `(` before directive arguments");

        while(!tok_is(p, TOKEN_RPAREN))
        {
            list_push(args, parser_peek(p, 0)->value);
            parser_consume(p, TOKEN_STRING, "expect string literal as directive argument");
            if(!tok_is(p, TOKEN_RPAREN))
                parser_consume(p, TOKEN_COMMA, "expect `,` between directive arguments");
        }

        parser_consume(p, TOKEN_RPAREN, "expect `)` after directive arguments");
    }

    if(directive->num_args != ANY && (size_t) directive->num_args != args->size) 
        throw_error(parser_context(p), ERR_UNDEFINED, name_tok, "directive `%s` expects %d arguments, got %lu", name, directive->num_args, args->size);

    return directive_data(directive, name_tok, args);
}

EVAL_FN(cc)
{
#ifdef CSPYDR_LINUX
    char* abs_path = get_absolute_path(data->name_token->source->path);
    char* working_dir = get_path_from_file(abs_path);
    if(!context->flags.silent) 
        LOG_OK_F(COLOR_BOLD_CYAN "  Command   " COLOR_RESET " \"%s %s\"\n", cc, (const char*) data->arguments->items[0]);
    List_T* args = init_list();
    list_push(args, cc);
    
    char* ch = strtok(data->arguments->items[0], " ");
    while(ch != NULL)
    {
        list_push(args, ch);
        ch = strtok(NULL, " ");
    }
    char current_dir[FILENAME_MAX];
    getcwd(current_dir, LEN(current_dir));
    chdir(working_dir);
    
    i32 exit_code = subprocess(cc, (char* const*) args->items, false);
    if(exit_code)
        throw_error(context, ERR_MISC, data->name_token, "command %s %s failed with exit code %d", cc, (const char*) data->arguments->items[0], exit_code);
    chdir(current_dir);
    free_list(args);
    free(abs_path);
#else
    throw_error(context, ERR_INTERNAL, data->name_token, "directive `cc` is only implemented for linux as of now");
#endif
    return false;
}

EVAL_FN(cfg)
{
    for(size_t i = 0; configurations[i].name; i++)
    {
        const Config_T* cfg = &configurations[i];
        if(strcmp(cfg->name, data->arguments->items[0]) == 0)
            return !cfg->set(context);
    }

    throw_error(context, ERR_UNDEFINED_UNCR, data->name_token, "undefined `cfg` directive `%s`", data->arguments->items[0]);
    return false;
}

EVAL_FN(copy)
{
#ifdef CSPYDR_LINUX
    char* from = data->arguments->items[0];
    char* to = data->arguments->items[0];

    if(!context->flags.silent) 
        LOG_OK_F(COLOR_BOLD_CYAN "  Command" COLOR_RESET "    \"cp -r %s %s\"\n", from, to);

    char* abs_path = get_absolute_path(data->name_token->source->path);
    char* working_dir = get_path_from_file(abs_path);
    char current_dir[FILENAME_MAX];
    getcwd(current_dir, LEN(current_dir));
    chdir(working_dir);

    char* const args[] = {
        "cp",
        "-r",
        from,
        to,
        NULL
    };
    i32 exit_code = subprocess(args[0], args, false);
    if(exit_code)
        throw_error(context, ERR_MISC, data->name_token, "copy failed with exit code %d", exit_code);

    chdir(current_dir);
    free(abs_path);
#else
    throw_error(context, ERR_INTERNAL, data->name_token, "directive `cc` is only implemented for linux as of now");
#endif
    return false;
}

EVAL_FN(drop)
{
    if(obj->args->size != 1)
    {
        throw_error(context, ERR_CALL_ERROR_UNCR, obj->tok, "function prefixed with `[drop]` has to have exactly one argument");
        return false;
    }

    ASTExitFnHandle_T* handle = mem_malloc(sizeof(ASTExitFnHandle_T));
    handle->fn = obj;
    handle->type = ((ASTObj_T*) obj->args->items[0])->data_type;
    handle->tok = data->name_token;

    if(!ast->type_exit_fns)
        mem_add_list(ast->type_exit_fns = init_list());
    list_push(ast->type_exit_fns, handle);

    return false;   
}

EVAL_FN(export)
{
    obj->exported = data->arguments->items[0];
    obj->referenced = true;
    return false;
}

EVAL_FN(link_dir)
{
    for(size_t i = 0; i < data->arguments->size; i++)
    {
        char* link_flag = calloc(strlen(data->arguments->items[i]) + 3, sizeof(char));
        sprintf(link_flag, "-L%s", (const char*) data->arguments->items[i]);
        mem_add_ptr(link_flag);

        list_push(context->linker_flags, link_flag);
    }
    return false;
}

EVAL_FN(link_obj)
{
    for(size_t i = 0; i < data->arguments->size; i++)
    {
        char* abs_path = get_absolute_path(data->name_token->source->path);
        char* working_dir = get_path_from_file(abs_path);

        char* full_fp = mem_malloc((strlen(working_dir) + strlen(DIRECTORY_DELIMS) + strlen(data->arguments->items[i]) + 2) * sizeof(char));
        sprintf(full_fp, "%s" DIRECTORY_DELIMS "%s", working_dir, (const char*) data->arguments->items[i]);
        list_push(context->linker_flags, full_fp);

        free(abs_path);
    }
    return false;
}

EVAL_FN(link)
{
    for(size_t i = 0; i < data->arguments->size; i++)
        pkg_config(context, data->arguments->items[i], data->name_token);
    return false;
}

EVAL_FN(no_return)
{
    obj->data_type->no_return = true;
    return false;
}

EVAL_FN(private)
{
    obj->private = true;
    return false;
}
