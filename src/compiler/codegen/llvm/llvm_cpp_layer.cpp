#include "llvm_cpp_layer.hpp"
#include "llvm_emitter.hpp"
#include "generators/llvm_generators.hpp"

#include <memory>

using namespace CSpydrLLVMCodegen; // this file can't be in the namespace, since it's the the "bridge" between C and C++

std::shared_ptr<LLVMCodegenData_T> CSpydrLLVMCodegen::codegen_data = std::make_shared<LLVMCodegenData_T>();

void llvm_gen_code(ASTProg_T* ast, bool silent, bool print_llvm)
{
    assert(codegen_data && "codegen_data is not initialized!");
    codegen_data->silent = silent;

    ProgramGenerator generator = ProgramGenerator(ast);
    generator.generate();

    if(print_llvm)
        generator.print_code();
}

void llvm_emit_code(const char* target, bool silent)
{
    assert(codegen_data && "codegen_data is not initialized!");
    codegen_data->silent = silent;

    LLVMEmitter emitter = LLVMEmitter(LLVMEmitType::BIN);
    emitter.emit(target);
}

void llvm_run_code(bool silent)
{
    assert(codegen_data && "codegen_data is not initialized!");
    codegen_data->silent = silent;

    LLVMEmitter emitter = LLVMEmitter(LLVMEmitType::JIT);
    emitter.emit();
}