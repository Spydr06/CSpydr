#ifndef CSPYDR_TIMER_H
#define CSPYDR_TIMER_H

#include "context.h"

void enable_timer(Context_T* context);
void disable_timer(Context_T* context);
void timer_start(Context_T* context, const char* description);
void timer_stop(Context_T* context);
void timer_print_summary(Context_T* context);

#endif