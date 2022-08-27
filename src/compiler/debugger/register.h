#ifndef CSPYDR_REGISTER_H
#define CSPYDR_REGISTER_H

#include <util.h>
#include <stdlib.h>
#include <globals.h>

typedef enum REGISTER_ENUM
{
    REG_RAX, REG_RBX, REG_RCX, REG_RDX,
    REG_RDI, REG_RSI, REG_RBP, REG_RSP,
    REG_R8,  REG_R9,  REG_R10, REG_R11,
    REG_R12, REG_R13, REG_R14, REG_R15,
    REG_RIP, REG_RFLAGS,       REG_CS,
    REG_ORIG_RAX, 
    REG_FS_BASE, REG_GS_BASE,
    REG_FS, REG_GS, REG_SS, REG_DS, REG_ES,
    REG_NUM
} Register_T;

typedef struct REG_DESCRIPTOR_STRUCT
{
    Register_T r;
    i32 dwarf_r;
    const char* name;
} RegDescriptor_T;

extern const RegDescriptor_T REG_DESCRIPTORS[REG_NUM];

const RegDescriptor_T* find_reg_desc(Register_T r);
u64 get_reg_val_from_dwarf(pid_t pid, u32 regnum);
u64 get_register_value(pid_t pid, Register_T r);
void set_register_value(pid_t pid, Register_T r, u64 value);
const char* get_register_name(Register_T r);
Register_T get_register_from_name(const char* name);
void dump_registers();

#endif