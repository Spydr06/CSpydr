#include "panic.h"
#include "context.h"

static PanicHandlerFn_T PANIC_HANDLER = default_panic_handler;

void set_panic_handler(PanicHandlerFn_T fn)
{
    PANIC_HANDLER = fn;
}

PanicHandlerFn_T get_panic_handler(void)
{
    return PANIC_HANDLER;
}

void panic(Context_T* context) {
    throw(context->main_error_exception);
}