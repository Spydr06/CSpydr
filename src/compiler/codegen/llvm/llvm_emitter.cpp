#include "llvm_emitter.hpp"
#include "../../io/log.h"

#include <iostream>
#include <assert.h>

namespace CSpydrLLVMCodegen 
{
    LLVMEmitter::LLVMEmitter(LLVMEmitType emit_type)
        : emit_type(emit_type)
    {
    }

    void LLVMEmitter::emit(const char* target)
    {
        switch(emit_type)
        {
            case LLVMEmitType::BIN:
                assert(target && "target is NULL, when emit_type is LLVMEmitType::BIN");
                emit_bin(target);
                break;
            case LLVMEmitType::JIT:
                emit_jit();
                break;
        }
    }

    void LLVMEmitter::emit_bin(const char* target)
    {
        if(!codegen_data->silent)
            LOG_OK_F(COLOR_BOLD_BLUE "  Building   " COLOR_RESET "%s\n", target);
    }

    void LLVMEmitter::emit_jit(void)
    {
        if(!codegen_data->silent)
            LOG_OK(COLOR_BOLD_BLUE "  Executing  " COLOR_RESET "using MC-JIT\n");
    }
}