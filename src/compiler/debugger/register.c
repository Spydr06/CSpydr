#include "register.h"
#include "debugger.h"
#include <string.h>
#include <io/log.h>
#include <sys/ptrace.h>
#include <sys/user.h>

const RegDescriptor_T REG_DESCRIPTORS[REG_NUM] = {
    {REG_R15, 15, "r15"},
    {REG_R14, 14, "r14"},
    {REG_R13, 13, "r13"},
    {REG_R12, 12, "r12"},
    {REG_RBP, 6,  "rbp"},
    {REG_RBX, 3,  "rbx"},
    {REG_R11, 11, "r11"},
    {REG_R10, 10, "r10"},
    {REG_R9,  9,  "r9" },
    {REG_R8,  8,  "r8" },
    {REG_RAX, 0,  "rax"},
    {REG_RCX, 2,  "rcx"},
    {REG_RDX, 1,  "rdx"},
    {REG_RSI, 4,  "rsi"},
    {REG_RDI, 5,  "rdi"},
    {REG_ORIG_RAX, -1, "orig_rax"},
    {REG_RIP, -1, "rip"},
    {REG_CS,  51, "cs" },
    {REG_RFLAGS, 49, "eflags"},
    {REG_RSP, 7,  "rsp"},
    {REG_SS,  52, "ss" },
    {REG_FS_BASE, 58, "fs_base"},
    {REG_GS_BASE, 59, "gs_base"},
    {REG_DS,  53, "ds" },
    {REG_ES,  50, "es" },
    {REG_FS,  54, "fs" },
    {REG_GS,  55, "gs" }
};

const RegDescriptor_T* find_reg_desc(Register_T r)
{
    for(size_t i = 0; i < REG_NUM; i++)
    {
        if(REG_DESCRIPTORS[i].r == r)
            return &REG_DESCRIPTORS[i];
    }
    
    debug_error("Unknown register ID %d.\n", r);
    return &REG_DESCRIPTORS[0];
}

u64 get_reg_val_from_dwarf(pid_t pid, u32 regnum)
{
    const RegDescriptor_T* it = NULL;
    for(size_t i = 0; i < REG_NUM; i++)
    {
        if(REG_DESCRIPTORS[i].dwarf_r == (i32) regnum)
        {
            it = &REG_DESCRIPTORS[i];
            break;
        }
    }

    if(it == &REG_DESCRIPTORS[REG_NUM - 1]) 
    {
        debug_error("Unknown dwarf register %d.", regnum);
        return 0;
    }

    return get_register_value(pid, it->r);
}

const char* get_register_name(Register_T r)
{
    for(size_t i = 0; i < REG_NUM; i++)
    {
        if(REG_DESCRIPTORS[i].r == r)
            return REG_DESCRIPTORS[i].name;
    }

    return NULL;
}

Register_T get_register_from_name(const char* name)
{
    for(size_t i = 0; i < REG_NUM; i++)
    {
        if(strcmp(REG_DESCRIPTORS[i].name, name) == 0)
            return REG_DESCRIPTORS[i].r;
    }

    return 0;
}

void dump_registers(Debugger_T* dbg) 
{
    for(size_t i = 0; i < REG_NUM; i++)
        fprintf(OUTPUT_STREAM, "%s%*s%016lx\n", 
            REG_DESCRIPTORS[i].name, 
            11 - (int) strlen(REG_DESCRIPTORS[i].name), 
            "0x", 
            get_register_value(dbg->loaded, REG_DESCRIPTORS[i].r)
        );
}

u64 get_register_value(pid_t pid, Register_T r)
{
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, pid, NULL, &regs);

    const RegDescriptor_T* it = find_reg_desc(r);

    return *(((u64*) &regs) + (it - REG_DESCRIPTORS));
}

void set_register_value(pid_t pid, Register_T r, u64 value)
{
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, pid, NULL, &regs);

    const RegDescriptor_T* it = find_reg_desc(r);

    *(((u64*) &regs) + (it - REG_DESCRIPTORS)) = value;

    ptrace(PTRACE_SETREGS, pid, NULL, &regs);
}