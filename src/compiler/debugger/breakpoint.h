#ifndef CSPYDR_BREAKPOINT_H
#define CSPYDR_BREAKPOINT_H

#include <globals.h>
#include <util.h>

#include "debugger.h"

typedef struct BREAKPOINT_STRUCT {
    pid_t pid;
    bool enabled;
    intptr_t addr;
    u8 saved_data;
} Breakpoint_T;

Breakpoint_T* init_breakpoint(Debugger_T* dbg, intptr_t addr);
void free_breakpoint(Breakpoint_T* brk);
void breakpoint_enable(Breakpoint_T* brk);
void breakpoint_disable(Breakpoint_T* brk);

Breakpoint_T* set_breakpoint_at_address(Debugger_T* dbg, intptr_t addr);
void disable_breakpoint_at_address(Debugger_T* dgb, intptr_t addr);
Breakpoint_T* find_breakpoint(Debugger_T* dbg, intptr_t addr);

#endif