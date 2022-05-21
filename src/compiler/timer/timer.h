#ifndef CSPYDR_TIMER_H
#define CSPYDR_TIMER_H

void enable_timer(void);
void disable_timer(void);
void timer_start(const char* description);
void timer_stop(void);
void timer_print_summary(void);

#endif