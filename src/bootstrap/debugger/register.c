#include "register.h"
#include "debugger.h"
#include <string.h>
#include <io/log.h>
#include <sys/ptrace.h>
#include <sys/user.h>

const RegDescriptor_T DEBUGGER_REG_DESCRIPTORS[DEBUGGER_REG_NUM] = {
    {DEBUGGER_REG_R15, 15, "r15"},
    {DEBUGGER_REG_R14, 14, "r14"},
    {DEBUGGER_REG_R13, 13, "r13"},
    {DEBUGGER_REG_R12, 12, "r12"},
    {DEBUGGER_REG_RBP, 6,  "rbp"},
    {DEBUGGER_REG_RBX, 3,  "rbx"},
    {DEBUGGER_REG_R11, 11, "r11"},
    {DEBUGGER_REG_R10, 10, "r10"},
    {DEBUGGER_REG_R9,  9,  "r9" },
    {DEBUGGER_REG_R8,  8,  "r8" },
    {DEBUGGER_REG_RAX, 0,  "rax"},
    {DEBUGGER_REG_RCX, 2,  "rcx"},
    {DEBUGGER_REG_RDX, 1,  "rdx"},
    {DEBUGGER_REG_RSI, 4,  "rsi"},
    {DEBUGGER_REG_RDI, 5,  "rdi"},
    {DEBUGGER_REG_ORIG_RAX, -1, "orig_rax"},
    {DEBUGGER_REG_RIP, -1, "rip"},
    {DEBUGGER_REG_CS,  51, "cs" },
    {DEBUGGER_REG_RFLAGS, 49, "eflags"},
    {DEBUGGER_REG_RSP, 7,  "rsp"},
    {DEBUGGER_REG_SS,  52, "ss" },
    {DEBUGGER_REG_FS_BASE, 58, "fs_base"},
    {DEBUGGER_REG_GS_BASE, 59, "gs_base"},
    {DEBUGGER_REG_DS,  53, "ds" },
    {DEBUGGER_REG_ES,  50, "es" },
    {DEBUGGER_REG_FS,  54, "fs" },
    {DEBUGGER_REG_GS,  55, "gs" }
};

const RegDescriptor_T* find_reg_desc(Register_T r)
{
    for(size_t i = 0; i < DEBUGGER_REG_NUM; i++)
    {
        if(DEBUGGER_REG_DESCRIPTORS[i].r == r)
            return &DEBUGGER_REG_DESCRIPTORS[i];
    }
    
    debug_error("Unknown register ID %d.\n", r);
    return &DEBUGGER_REG_DESCRIPTORS[0];
}

u64 get_reg_val_from_dwarf(pid_t pid, u32 regnum)
{
    const RegDescriptor_T* it = NULL;
    for(size_t i = 0; i < DEBUGGER_REG_NUM; i++)
    {
        if(DEBUGGER_REG_DESCRIPTORS[i].dwarf_r == (i32) regnum)
        {
            it = &DEBUGGER_REG_DESCRIPTORS[i];
            break;
        }
    }

    if(it == &DEBUGGER_REG_DESCRIPTORS[DEBUGGER_REG_NUM - 1]) 
    {
        debug_error("Unknown dwarf register %d.", regnum);
        return 0;
    }

    return get_register_value(pid, it->r);
}

const char* get_register_name(Register_T r)
{
    for(size_t i = 0; i < DEBUGGER_REG_NUM; i++)
    {
        if(DEBUGGER_REG_DESCRIPTORS[i].r == r)
            return DEBUGGER_REG_DESCRIPTORS[i].name;
    }

    return NULL;
}

Register_T get_register_from_name(const char* name)
{
    for(size_t i = 0; i < DEBUGGER_REG_NUM; i++)
    {
        if(strcmp(DEBUGGER_REG_DESCRIPTORS[i].name, name) == 0)
            return DEBUGGER_REG_DESCRIPTORS[i].r;
    }

    return 0;
}

void dump_registers(Debugger_T* dbg) 
{
    for(size_t i = 0; i < DEBUGGER_REG_NUM; i++)
        fprintf(OUTPUT_STREAM, "%s%*s%016lx\n", 
            DEBUGGER_REG_DESCRIPTORS[i].name, 
            11 - (int) strlen(DEBUGGER_REG_DESCRIPTORS[i].name), 
            "0x", 
            get_register_value(dbg->loaded, DEBUGGER_REG_DESCRIPTORS[i].r)
        );
}

u64 get_register_value(pid_t pid, Register_T r)
{
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, pid, NULL, &regs);

    const RegDescriptor_T* it = find_reg_desc(r);

    return *(((u64*) &regs) + (it - DEBUGGER_REG_DESCRIPTORS));
}

void set_register_value(pid_t pid, Register_T r, u64 value)
{
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, pid, NULL, &regs);

    const RegDescriptor_T* it = find_reg_desc(r);

    *(((u64*) &regs) + (it - DEBUGGER_REG_DESCRIPTORS)) = value;

    ptrace(PTRACE_SETREGS, pid, NULL, &regs);
}
