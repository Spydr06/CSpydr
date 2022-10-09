#include "panic.h"

#include <io/log.h>
#include <globals.h>

void linter_panic_handler(void)
{
    // Emit an error summary
    if(!global.silent)
    {
        if(global.emitted_errors && global.emitted_warnings)
            LOG_ERROR_F(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " %u error%s and %u warning%s thrown during code validation; aborting.\n", global.emitted_errors, global.emitted_errors == 1 ? "" : "s", global.emitted_warnings, global.emitted_warnings == 1 ? "" : "s");
        else if(global.emitted_errors)
            LOG_ERROR_F(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " %u error%s thrown during code validation; aborting.\n", global.emitted_errors, global.emitted_errors == 1 ? "" : "s");
        else if(global.emitted_warnings)
            LOG_WARN_F(COLOR_BOLD_YELLOW "[Warning]" COLOR_RESET COLOR_YELLOW " %u warning%s thrown during code validation\n", global.emitted_warnings, global.emitted_warnings == 1 ? "" : "s");
    }
}