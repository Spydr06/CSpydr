#include "breakpoint.h"

#include <sys/ptrace.h>
#include "dbg.h"

Breakpoint_T* init_breakpoint(intptr_t addr)
{
    Breakpoint_T* brk = calloc(1, sizeof(struct BREAKPOINT_STRUCT));
    brk->addr = addr;
    brk->pid = global.debugger.loaded;

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

Breakpoint_T* set_breakpoint_at_address(intptr_t addr)
{
    Breakpoint_T* found = NULL;
    for(size_t i = 0; i < global.debugger.breakpoints->size; i++)
    {
        Breakpoint_T* b = global.debugger.breakpoints->items[i];
        if(b->addr == addr) 
        {
            found = b;
            break;
        }
    }

    if(found)
    {
        if(!found->enabled)
            breakpoint_enable(found);

        return found;
    }
    else
    {
        Breakpoint_T* brk = init_breakpoint(addr);
        breakpoint_enable(brk);
        list_push(global.debugger.breakpoints, brk);

        return brk;
    }
}

void disable_breakpoint_at_address(intptr_t addr)
{
    Breakpoint_T* found = NULL;
    for(size_t i = 0; i < global.debugger.breakpoints->size; i++)
    {
        Breakpoint_T* b = global.debugger.breakpoints->items[i];
        if(b->addr == addr) 
        {
            found = b;
            break;
        }
    }

    if(found)
        breakpoint_disable(found);
    else
        debug_error("No breakpoint enabled at address 0x%016lx.\n", addr);
}