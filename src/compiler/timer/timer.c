#include "timer.h"
#include "globals.h"
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

void enable_timer(void) {
    global.timer_enabled = true;

    if(!global.timesteps) {
        global.timesteps = init_list();
    }
}

void disable_timer(void) {
    global.timer_enabled = false;
}

void timer_start(const char* description) {
    if(!global.timer_enabled)
        return;

    TimeStep_T* ts = malloc(sizeof(TimeStep_T));
    memset(ts, 0, sizeof(TimeStep_T));

    ts->description = description;
    ts->begin = clock();
    
    list_push(global.timesteps, ts);
}

void timer_stop(void) {
    if(!global.timer_enabled)
        return;

    if(!global.timesteps || global.timesteps->size == 0) {
        LOG_WARN_F(COLOR_BOLD_YELLOW "[Warning]" COLOR_RESET COLOR_YELLOW " " __FILE__ ":%d: called %s while no timer is running", __LINE__, FUNC);
    } 

    ((TimeStep_T*) global.timesteps->items[global.timesteps->size - 1])->end = clock();
}

void timer_print_summary(void)
{
    if(!global.timer_enabled)
        return;
    
    if(!global.timesteps) {
        LOG_WARN_F(COLOR_BOLD_YELLOW "[Warning]" COLOR_RESET COLOR_YELLOW " " __FILE__ ":%d: called %s while no timer was initialized", __LINE__, FUNC);
    } 
    
    LOG_INFO_F(COLOR_BOLD_WHITE "[Timer]" COLOR_RESET " %lu recorded timings:\n", global.timesteps->size);

    double total = 0.0;

    for(size_t i = 0; i < global.timesteps->size; i++) {
        TimeStep_T* ts = global.timesteps->items[i];
        double duration = (double)(ts->end - ts->begin) / CLOCKS_PER_SEC * 1000;
        total += duration;

        LOG_INFO_F("  %.03lfms\t%s\n", duration, ts->description);
    }

    LOG_INFO_F(
        "==========================================\n" 
        COLOR_BOLD_WHITE 
        "  total: %lu lines in %.03lfms (%.03lfs)\n"
        COLOR_RESET, 
        global.total_source_lines, total, total / 1000
    );
}