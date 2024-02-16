#include "linker.h"
#include "config.h"
#include "context.h"
#include "error/error.h"
#include "io/io.h"
#include "io/log.h"
#include "list.h"
#include "platform/linux/linux_platform.h"
#include "timer/timer.h"

#include <string.h>
#include <libgen.h>

static void print_linking_msg(Context_T* context, const char* target, bool link_exec) 
{
    LOG_OK_F(
        COLOR_BOLD_BLUE "  Linking   " COLOR_RESET " %s " COLOR_BOLD_WHITE "(%s; %s)" COLOR_RESET,
        basename((char*) target),
        link_exec ? "executable" : "library",
        context->link_mode.mode == LINK_DYNAMIC ? "dynamic" : "static"
    );

    if(context->link_mode.libs->size > 0)
    {
        LOG_OK(COLOR_RESET " (");
        for(size_t i = 0; i < context->link_mode.libs->size; i++) 
        {
            char* lib = context->link_mode.libs->items[i];
            if(lib[0] == '-' && lib[1] == 'l') {
                LOG_OK_F(COLOR_RESET "%s%s", (char*) lib + 2, context->link_mode.libs->size - i <= 1 ? ")" : ", ");
            }
        }
    }
    LOG_OK(COLOR_RESET "\n");
}

static const char* link_get_arch(Target_T* target)
{
    if(target->platform == PLATFORM_WINDOWS)
    {
        printf("WINDOWS LINKING NOT IMPLEMENTED YET\n");
        exit(2);
    }

    switch(target->arch)
    {
    case ARCH_X86_64:
        return "elf_x86_64";
    case ARCH_AARCH64:
        return "elf_aarch64";
    case ARCH_RISCV64:
        return "elf_riscv64";
    default:
        unreachable();
        return "unkown";
    }
}

static List_T* common_linker_args(Context_T* context, const char* target_filepath, const char* object_filepath)
{
    List_T* args = init_list();
    list_push(args, context->ld);
    
    list_push(args, (void*) object_filepath);

    list_push(args, "-o");
    list_push(args, (void*) target_filepath);
    list_push(args, "-m");
    list_push(args, (void*) link_get_arch(&context->target));

    if(context->target.arch == ARCH_X86_64)
    {
        list_push(args, "-L/usr/lib64");
        list_push(args, "-L/lib64");
    }
    
    list_push(args, "-L/usr/lib");
    list_push(args, "-L/lib");

    return args;
}

static void dynamic_linker_args(Context_T* context, List_T* args)
{
    const char* dynamic_linker = context->link_mode.ldynamic.dynamic_linker;
    if(!file_exists(dynamic_linker))
    {
        context->emitted_warnings++;
        LOG_WARN_F(COLOR_BOLD_YELLOW "[Warning]" COLOR_RESET COLOR_YELLOW " dynamic linker `%s` does not exist.\n", dynamic_linker);
    }

    list_push(args, "-dynamic-linker");
    list_push(args, (void*) dynamic_linker);
}

static void static_linker_args(Context_T* context, List_T* args)
{
    list_push(args, "-static");
}

static i32 link_executable(Context_T* context, const char* target_filepath, const char* object_filepath)
{
    List_T* args = common_linker_args(context, target_filepath, object_filepath);
    
    bool use_libc = 
        context->target.platform != PLATFORM_FREESTANDING && 
        context->target.platform != PLATFORM_WINDOWS;
    
    if(use_libc)
    {
        if(context->target.arch == ARCH_X86_64)
        {
            list_push(args, "/usr/lib64/crt1.o");
            list_push(args, "/usr/lib64/crti.o");
            list_push(args, "/usr/lib64/crtn.o");
        }
        else
        {
            list_push(args, "/usr/lib/crt1.o");
            list_push(args, "/usr/lib/crti.o");
            list_push(args, "/usr/lib/crtn.o");
        }

        list_push(args, "-lc");
    }

    switch(context->link_mode.mode)
    {
    case LINK_DYNAMIC:
        dynamic_linker_args(context, args);
        break;
    case LINK_STATIC:
        static_linker_args(context, args);
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

static i32 link_library(Context_T* context, const char* target_filepath, const char* object_filepath)
{
    List_T* args = common_linker_args(context, target_filepath, object_filepath);
    list_push(args, "-shared");

    if(context->link_mode.mode == LINK_STATIC)
        static_linker_args(context, args);

    for(size_t i = 0; i < context->link_mode.extra->size; i++)
        list_push(args, context->link_mode.extra->items[i]);

    for(size_t i = 0; i < context->link_mode.libs->size; i++)
        list_push(args, context->link_mode.libs->items[i]);

    list_push(args, NULL);

    i32 exit_code = subprocess((char*) args->items[0], (char* const*) args->items, false);
    free_list(args);
    return exit_code;
}

i32 linker_pass(Context_T* context, const char* target_filepath, const char* object_filepath)
{
    timer_start(context, "linking");

    bool link_exec = context->flags.require_entrypoint; // TODO: add better way to find this out

    if(!context->flags.silent)
        print_linking_msg(context, target_filepath, link_exec);

    i32 exit_code = (link_exec ? link_executable : link_library)(context, target_filepath, object_filepath);
    if(exit_code != 0)
    {
        LOG_ERROR_F(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " Linker terminated with non-zero exit code %d.\n", exit_code);
        throw(context->main_error_exception);
    }

    timer_stop(context);
    return 0;
}

