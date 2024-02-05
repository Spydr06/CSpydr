#include "linker.h"

#include "config.h"
#include "error/error.h"
#include "io/io.h"
#include "io/log.h"
#include "list.h"
#include "timer/timer.h"
#include "platform/platform_bindings.h"
#include "codegen/codegen_utils.h"

static int link_exec(Context_T* context, const char* target, char* obj_file);
static int link_lib(Context_T* context, const char* target, char* obj_file);

void link_obj(Context_T* context, const char* target, char* obj_file, bool silent, bool _link_exec)
{
    timer_start(context, "linking");
    if(!silent)
        print_linking_msg(context, target, _link_exec);

    i32 exit_code = (_link_exec ? link_exec : link_lib)(context, target, obj_file);
    if(exit_code != 0)
    {
        LOG_ERROR_F("error linking code. (exit code %d)\n", exit_code);
        throw(context->main_error_exception);
    }

    timer_stop(context);
}

static void dynamic_linker_flags(Context_T* context, List_T* args) {
    const char* dynamic_linker = context->link_mode.ldynamic.dynamic_linker;
    if(!file_exists(dynamic_linker))
    {
        context->emitted_warnings++;
        LOG_WARN_F(COLOR_BOLD_YELLOW "[Warning]" COLOR_RESET COLOR_YELLOW " dynamic linker `%s` does not exist.\n", dynamic_linker);
    }

    list_push(args, "-dynamic-linker");
    list_push(args, (void*) dynamic_linker);
}

static void static_linker_flags(Context_T* context, List_T* args)
{
    list_push(args, "-static");
}

static int link_exec(Context_T* context, const char* target, char* obj_file) {
    List_T* args = init_list();
    list_push(args, DEFAULT_LINKER);
    list_push(args, obj_file);
    list_push(args, "-o");
    list_push(args, (void*) target);
    list_push(args, "-m");
    list_push(args, "elf_x86_64");
    list_push(args, "-L/usr/lib64");
    list_push(args, "-L/lib64");
    list_push(args, "-L/usr/lib");
    list_push(args, "-L/lib");
 
    switch(context->link_mode.mode)
    {
        case LINK_DYNAMIC:
            dynamic_linker_flags(context, args);
            break;
        case LINK_STATIC:
            static_linker_flags(context, args);
            break;
        default:
            unreachable();
            break;
    }

    for(size_t i = 0; i < context->link_mode.extra->size; i++)
        list_push(args, context->link_mode.extra->items[i]);

    for(size_t i = 0; i < context->link_mode.libs->size; i++)
        list_push(args, context->link_mode.libs->items[i]);

    list_push(args, NULL);

    i32 exit_code = subprocess((char*) args->items[0], (char* const*) args->items, false);
    free_list(args);
    return exit_code;    
}

static int link_lib(Context_T* context, const char* target, char* obj_file)
{
    List_T* args = init_list();
    list_push(args, DEFAULT_LINKER);
    list_push(args, "-o");
    list_push(args, (void*) target);
    list_push(args, "-m");
    list_push(args, "elf_x86_64");
    list_push(args, "-L/usr/lib64");
    list_push(args, "-L/lib64");
    list_push(args, "-L/usr/lib");
    list_push(args, "-L/lib");
    list_push(args, "-shared");

    if(context->link_mode.mode == LINK_STATIC)
        list_push(args, "-static");

    for(size_t i = 0; i < context->link_mode.extra->size; i++)
        list_push(args, context->link_mode.extra->items[i]);

    for(size_t i = 0; i < context->link_mode.libs->size; i++)
        list_push(args, context->link_mode.libs->items[i]);

    list_push(args, obj_file);
    list_push(args, NULL);

    i32 exit_code = subprocess((char*) args->items[0], (char* const*) args->items, false);
    free_list(args);
    return exit_code;    
}
