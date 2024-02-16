#ifndef CSPYDR_CODEGEN_H
#define CSPYDR_CODEGEN_H

#include "codegen/backend.h"
#include "config.h"
#include "context.h"
#include "ir/ir.h"

#include <stdio.h>

typedef struct CODEGEN_DATA_STRUCT {
    Context_T* context;
    IR_T* ir;

    struct {
        char* buf;
        size_t buf_len;
        FILE* code_buffer;
    } output;

    bool generate_debug_info;
    const BackendCallbacks_T* b;
} CodegenData_T;

#ifdef __GNUC__
__attribute__((format(printf, 2, 3)))
#endif
void codegen_printf(CodegenData_T* c, const char* fmt, ...);

#ifdef __GNUC__
__attribute__((format(printf, 2, 3)))
#endif
void codegen_println(CodegenData_T* c, const char* fmt, ...);

#define CODEGEN_WRITE(c, val) _Generic(val,     \
        u8: codegen_write_u8(c, val)  ,         \
        u16: codegen_write_u16(c, val),         \
        u32: codegen_write_u32(c, val),         \
        u64: codegen_write_u64(c, val),         \
        char*: codegen_printf(c, "%s", val),    \
    )

void codegen_write_u8(CodegenData_T* c, u8 val);
void codegen_write_u16(CodegenData_T* c, u16 val);
void codegen_write_u32(CodegenData_T* c, u32 val);
void codegen_write_u64(CodegenData_T* c, u64 val);

i32 codegen_pass(Context_T* context, IR_T* ir);

#endif

