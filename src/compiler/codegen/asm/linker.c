#include "linker.h"

#include "io/log.h"
#include "globals.h"
#include "timer/timer.h"
#include "platform/platform_bindings.h"
#include "codegen/codegen_utils.h"

void link_obj(const char* target, char* obj_file, bool silent)
{
    timer_start("linking");

    if(!silent)
        print_linking_msg(target);

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
    
        for(size_t i = 0; i < global.linker_flags->size; i++)
            list_push(args, global.linker_flags->items[i]);
    
        list_push(args, "-dynamic-linker");
        list_push(args, "/lib64/ld-linux-x86-64.so.2");
        list_push(args, obj_file);
        list_push(args, NULL);
    
    
        i32 exit_code = subprocess((char*) args->items[0], (char* const*) args->items, false);
        if(exit_code != 0)
        {
            LOG_ERROR_F("error linking code. (exit code %d)\n", exit_code);
            throw(global.main_error_exception);
        }
    
        free_list(args);
    }

    timer_stop();
}
