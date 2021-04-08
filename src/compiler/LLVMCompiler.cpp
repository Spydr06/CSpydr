#include "LLVMIncludes.hpp"
#include "LLVMCompiler.hpp"
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <memory>
#include <vector>
#include "../log.h"

namespace CSpydr
{
    LLVMCompiler::LLVMCompiler(std::string targetPath, std::string moduleName, bool emitDebugInfo)
        : targetPath(targetPath), moduleName(moduleName), emitDebugInfo(emitDebugInfo)
    {
        llvmContext = std::make_unique<llvm::LLVMContext>();
        
        llvmModule = std::make_unique<llvm::Module>(llvm::StringRef(moduleName), *llvmContext);
        llvmBuilder = std::make_unique<llvm::IRBuilder<>>(*llvmContext);
    }

    LLVMCompiler::~LLVMCompiler()
    {
    }

    void LLVMCompiler::compile(ASTRoot_T* ast)
    {
        LOG_OK_F(COLOR_BOLD_GREEN "Compiling" COLOR_RESET " module \"%s\" to file \"%s\"\n", moduleName.c_str(), targetPath.c_str());

        for(size_t i = 0; i < ast->globals->size; i++)
        {
            this->generateGlobalVar((ASTGlobal_T*) ast->globals->items[i]);
        }

        for(size_t i = 0; i < ast->functions->size; i++)
        {
            this->generateFunction((ASTFunction_T*) ast->functions->items[i]);
        }

        if(emitDebugInfo) 
        {
            llvmModule->print(llvm::errs(), nullptr);
        }
    }

    llvm::GlobalVariable* LLVMCompiler::generateGlobalVar(ASTGlobal_T* ASTGlobal)
    {
        llvm::GlobalVariable* global = new llvm::GlobalVariable(*this->llvmModule, generateLLVMType(ASTGlobal->dataType), false, llvm::GlobalValue::CommonLinkage, 0, ASTGlobal->name);
        return global;
    }

    llvm::Function* LLVMCompiler::generateFunction(ASTFunction_T* ast)
    {
        llvm::Function* function = this->llvmModule->getFunction(ast->name);    //check if the function is already defined

        if(!function) 
        {
            function = createFunction(ast);
        }

        if(!function)
        {
            return nullptr;
        }

        this->generateCompound(ast->body, function);

        llvm::verifyFunction(*function);
        return function;
    }

    llvm::Function* LLVMCompiler::createFunction(ASTFunction_T* ast)
    {
        std::vector<llvm::Type*> argumentTypes;
        for(size_t i = 0; i < ast->args->size; i++)
        {
            argumentTypes.insert(argumentTypes.end(), generateLLVMType(((ASTArgument_T*) ast->args->items[i])->dataType));
        }

        llvm::FunctionType* ft = llvm::FunctionType::get(generateLLVMType(ast->returnType), argumentTypes ,false);
        llvm::Function* f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, ast->name, llvmModule.get());

        size_t idx = 0;
        for(auto &arg : f->args())
        {
            arg.setName(((ASTArgument_T*) ast->args->items[idx++])->name);
        }

        return f;
    }

    llvm::BasicBlock* LLVMCompiler::generateCompound(ASTCompound_T* ast, llvm::Function* functionRef, std::string name)
    {
        llvm::BasicBlock* bb = llvm::BasicBlock::Create(*this->llvmContext, name, functionRef);
        this->llvmBuilder->SetInsertPoint(bb);

        this->namedValues.clear();
        for(auto &arg : functionRef->args())
        {
            namedValues[std::string(arg.getName())] = &arg;
        }

        for(size_t i = 0; i < ast->body->size; i++)
        {
            ASTCompoundInstruction_T* ins = (ASTCompoundInstruction_T*) ast->body->items[i];

            switch(ins->type)
            {
                case AST_CI_LOCALDEF:
                    this->generateLocalVar((ASTLocal_T*) ins->ptr, bb);
                    break;

                case AST_CI_FN_CALL:
                    this->generateFunctionCall((ASTExprFnCall_T*) ins->ptr);
                    break;

                case AST_CI_RETURN_STMT:
                    

                default:
                    break;
            }
        }

        return bb;
    }

    llvm::AllocaInst* LLVMCompiler::generateLocalVar(ASTLocal_T* ast, llvm::BasicBlock* surroundingBlock)
    {
        if(namedValues[ast->name])
        {
            LOG_ERROR_F("local variable with the name \"%s\" already exists in current scope\n", ast->name);
            exit(1);
        }

        return new llvm::AllocaInst(generateLLVMType(ast->dataType), 0, ast->name, surroundingBlock);
    }

    llvm::Value* LLVMCompiler::generateVariableCall(ASTExprVarCall_T* ast)
    {
        return namedValues[ast->callee];
    }

    llvm::Value* LLVMCompiler::generateFunctionCall(ASTExprFnCall_T* ast)
    {
        return nullptr;
    }

    llvm::Type* LLVMCompiler::generateLLVMType(ASTDataType_T* ast)
    {
        switch(ast->basicType)
        {
            case AST_I32:
                return llvm::Type::getInt32Ty(*this->llvmContext);
            case AST_I64:
                return llvm::Type::getInt64Ty(*this->llvmContext);
            case AST_U32:
                return llvm::Type::getInt64Ty(*this->llvmContext);
            case AST_U64:
                return llvm::Type::getInt128Ty(*this->llvmContext);
            
            case AST_F32:
                return llvm::Type::getFloatTy(*this->llvmContext);
            case AST_F64:
                return llvm::Type::getDoubleTy(*this->llvmContext);

            case AST_BOOL:
                return llvm::Type::getInt1Ty(*this->llvmContext);
            case AST_VOID:
                return llvm::Type::getVoidTy(*this->llvmContext);

            case AST_CHAR:
                return llvm::Type::getInt8Ty(*this->llvmContext);
            case AST_STR:
                return llvm::ArrayType::get(llvm::Type::getInt8Ty(*this->llvmContext), 0);

            case AST_ARRAY:
                return llvm::ArrayType::get(generateLLVMType(ast->innerType), ast->numberOfIndices == NULL ? 0 : *((int*)ast->numberOfIndices->value));

            default:
                LOG_ERROR_F("variable of type \"%d\" is currently not supported\n",  ast->basicType);
                exit(1);
        }

        return nullptr;
    }
}