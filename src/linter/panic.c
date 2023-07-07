#include "panic.h"

#include <io/log.h>

void linter_panic_handler(Context_T* context)
{
    // Emit an error summary
    if(!context->flags.silent)
    {
        if(context->emitted_errors && context->emitted_warnings)
            LOG_ERROR_F(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " %u error%s and %u warning%s thrown during code validation; aborting.\n", context->emitted_errors, context->emitted_errors == 1 ? "" : "s", context->emitted_warnings, context->emitted_warnings == 1 ? "" : "s");
        else if(context->emitted_errors)
            LOG_ERROR_F(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " %u error%s thrown during code validation; aborting.\n", context->emitted_errors, context->emitted_errors == 1 ? "" : "s");
        else if(context->emitted_warnings)
            LOG_WARN_F(COLOR_BOLD_YELLOW "[Warning]" COLOR_RESET COLOR_YELLOW " %u warning%s thrown during code validation\n", context->emitted_warnings, context->emitted_warnings == 1 ? "" : "s");
    }
}