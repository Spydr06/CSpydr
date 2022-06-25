#ifndef CSPYDR_PANIC_H
#define CSPYDR_PANIC_H

#include "exception.h"
#include "globals.h"

typedef void (*PanicHandlerFn_T)(void);

void panic(void);
void set_panic_handler(PanicHandlerFn_T fn);
PanicHandlerFn_T get_panic_handler(void);
void default_panic_handler(void);

#endif