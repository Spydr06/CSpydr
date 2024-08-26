#ifndef CSPYDR_IR_DEBUG_H
#define CSPYDR_IR_DEBUG_H

#ifndef NDEBUG

#include "ir/ir.h"

#include <stdint.h>

typedef enum : u8 {
    IR_PRINT_FUNC           = 0b001,
    IR_PRINT_EXTERN_FUNC    = 0b010,
    IR_PRINT_GLOBAL         = 0b100,
    IR_PRINT_ANY            = UINT8_MAX,
} IRDebugFilter_T;

void dbg_print_ir(IR_T* ir, IRDebugFilter_T filter);
void dbg_print_ir_type(IRType_T* type);

void dbg_print_ir_lvalue(IRLValue_T* lvalue);
void dbg_print_ir_literal(IRLiteral_T* expr);
void dbg_print_ir_stmt(IRStmt_T* stmt);

void dbg_print_ir_function(IRFunction_T* func, IRDebugFilter_T filter);
void dbg_print_ir_global(IRGlobal_T* global, IRDebugFilter_T filter);

#endif

#endif

