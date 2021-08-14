#pragma once

#include "llvm_c_bindings.h"

class LLVMCodegen {
    public:
    LLVMCodegen(ASTProg_T* ast);

    private:
    ASTProg_T* ast;
};