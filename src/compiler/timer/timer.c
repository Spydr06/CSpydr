#include "timer.h"
#include "io/log.h"
#include "list.h"

#include <stdbool.h>
#include <time.h>
#include <memory.h>

#define FUNC __func__

typedef struct {
    const char* description;
    clock_t begin;
    clock_t end;
} TimeStep_T;

void enable_timer(Context_T* context) {
    context->flags.timer_enabled = true;

    if(!context->timesteps) {
        context->timesteps = init_list();
    }
}

void disable_timer(Context_T* context) {
    context->flags.timer_enabled = false;
}

void timer_start(Context_T* context, const char* description) {
    if(!context->flags.timer_enabled)
        return;

    TimeStep_T* ts = malloc(sizeof(TimeStep_T));
    memset(ts, 0, sizeof(TimeStep_T));

    ts->description = description;
    ts->begin = clock();
    
    list_push(context->timesteps, ts);
}

void timer_stop(Context_T* context) {
    if(!context->flags.timer_enabled)
        return;

    if(!context->timesteps || context->timesteps->size == 0) {
        LOG_WARN_F(COLOR_BOLD_YELLOW "[Warning]" COLOR_RESET COLOR_YELLOW " " __FILE__ ":%d: called %s while no timer is running", __LINE__, FUNC);
    } 

    ((TimeStep_T*) context->timesteps->items[context->timesteps->size - 1])->end = clock();
}

void timer_print_summary(Context_T* context)
{
    if(!context->flags.timer_enabled)
        return;
    
    if(!context->timesteps) {
        LOG_WARN_F(COLOR_BOLD_YELLOW "[Warning]" COLOR_RESET COLOR_YELLOW " " __FILE__ ":%d: called %s while no timer was initialized", __LINE__, FUNC);
    } 
    
    LOG_INFO_F(COLOR_BOLD_WHITE "[Timer]" COLOR_RESET " %lu recorded timings:\n", context->timesteps->size);

    double total = 0.0;

    for(size_t i = 0; i < context->timesteps->size; i++) {
        TimeStep_T* ts = context->timesteps->items[i];
        double duration = (double)(ts->end - ts->begin) / CLOCKS_PER_SEC * 1000;
        total += duration;

        LOG_INFO_F("  %.03lfms\t%s\n", duration, ts->description);
    }

    LOG_INFO_F(
        "==========================================\n" 
        COLOR_BOLD_WHITE 
        "  total: %lu lines in %.03lfms (%.03lfs)\n"
        COLOR_RESET, 
        context->total_source_lines, total, total / 1000
    );
}
