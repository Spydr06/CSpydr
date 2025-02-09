#ifndef CSPYDR_PANIC_H
#define CSPYDR_PANIC_H

#include "config.h"
#include "exception.h"

typedef void (*PanicHandlerFn_T)(Context_T*);

void panic(Context_T* context);
void set_panic_handler(PanicHandlerFn_T fn);
PanicHandlerFn_T get_panic_handler(void);
void default_panic_handler(Context_T*);

#endif