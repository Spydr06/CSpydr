#include "breakpoint.h"

#include <sys/ptrace.h>
#include "debugger.h"
#include "debugger/register.h"

Breakpoint_T* init_breakpoint(Debugger_T* dbg, intptr_t addr)
{
    Breakpoint_T* brk = calloc(1, sizeof(struct BREAKPOINT_STRUCT));
    brk->addr = addr;
    brk->pid = dbg->loaded;

    return brk;
}

void free_breakpoint(Breakpoint_T* brk)
{
    free(brk);
}

void breakpoint_enable(Breakpoint_T* brk)
{
    u64 data = ptrace(PTRACE_PEEKDATA, brk->pid, brk->addr, NULL);
    brk->saved_data = data & 0xff;

    u64 int3 = ((data & ~0xff) | 0xcc); // set bottom byte to 0xcc

    ptrace(PTRACE_POKEDATA, brk->pid, brk->addr, int3);
    brk->enabled = true;
    if(!global.silent)
        debug_info("Enabled breakpoint at address 0x%016lx", brk->addr);
}

void breakpoint_disable(Breakpoint_T* brk)
{
    u64 data = ptrace(PTRACE_PEEKDATA, brk->pid, brk->addr, NULL);
    u64 restored_data = ((data & ~0xff) | brk->saved_data);
    ptrace(PTRACE_POKEDATA, brk->pid, brk->addr, restored_data);

    brk->enabled = false;
    if(!global.silent)
        debug_info("Disabled breakpoint at address 0x%016lx", brk->addr);
}

Breakpoint_T* find_breakpoint(Debugger_T* dbg, intptr_t addr)
{
    for(size_t i = 0; i < dbg->breakpoints->size; i++)
    {
        Breakpoint_T* b = dbg->breakpoints->items[i];
        if(b->addr == addr) 
            return b;
    }

    return NULL;
}

Breakpoint_T* set_breakpoint_at_address(Debugger_T* dbg, intptr_t addr)
{
    Breakpoint_T* found = find_breakpoint(dbg, addr);

    if(found)
    {
        if(!found->enabled)
            breakpoint_enable(found);

        return found;
    }
    else
    {
        Breakpoint_T* brk = init_breakpoint(dbg, addr);
        breakpoint_enable(brk);
        list_push(dbg->breakpoints, brk);

        return brk;
    }
}

void disable_breakpoint_at_address(Debugger_T* dbg, intptr_t addr)
{
    Breakpoint_T* found = find_breakpoint(dbg, addr);

    if(found)
        breakpoint_disable(found);
    else
        debug_error("No breakpoint enabled at address 0x%016lx.\n", addr);
}