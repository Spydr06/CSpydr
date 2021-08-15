#pragma once

#include <memory>
#include "llvm_cpp_layer.hpp"

namespace CSpydrLLVMCodegen 
{
    #define CSP_LLVM_JIT_NAME "MC-JIT"

    enum LLVMEmitType {
        BIN,    // compile a binary
        JIT     // run the code in MC-JIT
    };

    class LLVMEmitter {
    public:
        LLVMEmitter(LLVMEmitType emit_type);
        ~LLVMEmitter() {};

        void emit(const char* target = nullptr);
    private:
        LLVMEmitType emit_type;

        void emit_bin(const char* target);
        void emit_jit(void);
    };
}