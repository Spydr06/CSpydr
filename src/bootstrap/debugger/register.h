#ifndef CSPYDR_REGISTER_H
#define CSPYDR_REGISTER_H

#include <util.h>

#include "debugger.h"

typedef enum REGISTER_ENUM
{
    DEBUGGER_REG_RAX, DEBUGGER_REG_RBX, DEBUGGER_REG_RCX, DEBUGGER_REG_RDX,
    DEBUGGER_REG_RDI, DEBUGGER_REG_RSI, DEBUGGER_REG_RBP, DEBUGGER_REG_RSP,
    DEBUGGER_REG_R8,  DEBUGGER_REG_R9,  DEBUGGER_REG_R10, DEBUGGER_REG_R11,
    DEBUGGER_REG_R12, DEBUGGER_REG_R13, DEBUGGER_REG_R14, DEBUGGER_REG_R15,
    DEBUGGER_REG_RIP, 
    DEBUGGER_REG_RFLAGS,       DEBUGGER_REG_CS,
    DEBUGGER_REG_ORIG_RAX, 
    DEBUGGER_REG_FS_BASE, DEBUGGER_REG_GS_BASE,
    DEBUGGER_REG_FS, DEBUGGER_REG_GS, DEBUGGER_REG_SS, DEBUGGER_REG_DS, DEBUGGER_REG_ES,
    DEBUGGER_REG_NUM
} Register_T;

typedef struct DEBUGGER_REG_DESCRIPTOR_STRUCT
{
    Register_T r;
    i32 dwarf_r;
    const char* name;
} RegDescriptor_T;

extern const RegDescriptor_T DEBUGGER_REG_DESCRIPTORS[DEBUGGER_REG_NUM];

const RegDescriptor_T* find_reg_desc(Register_T r);
u64 get_reg_val_from_dwarf(pid_t pid, u32 regnum);
u64 get_register_value(pid_t pid, Register_T r);
void set_register_value(pid_t pid, Register_T r, u64 value);
const char* get_register_name(Register_T r);
Register_T get_register_from_name(const char* name);
void dump_registers(Debugger_T* dbg);

#endif
