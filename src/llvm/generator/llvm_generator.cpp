#include "llvm_generator.hpp"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include <memory>
#include "../../io/log.h"

namespace CSpydr 
{
    LLVMGenerator::LLVMGenerator(std::string module)
        : moduleName(module)
    {
        LLVMContext = std::make_unique<llvm::LLVMContext>();

        llvmModule = std::make_unique<llvm::Module>(llvm::StringRef(module), *LLVMContext);
        llvmBuilder = std::make_unique<llvm::IRBuilder<>>(*LLVMContext);
    }

    LLVMGenerator::~LLVMGenerator()
    {
    }

    LLVMGenerator::status LLVMGenerator::generate(ASTRoot_T *ast)
    {
        for(size_t i = 0; i < ast->globals->size; i++)
        {
            generateGlobalVar((ASTGlobal_T*) ast->globals->items[i]);
        }

        for(size_t i = 0; i < ast->functions->size; i++)
        {
            generateFunction((ASTFunction_T*) ast->functions->items[i]);
        }

        llvmModule->print(llvm::errs(), nullptr);
        return STATUS_OK;
    }

    llvm::Type* LLVMGenerator::generateType(ASTType_T* type)
    {
        switch(type->type)
        {
            case AST_I32:
                return llvm::Type::getInt32Ty(*LLVMContext);
            case AST_I64:
                return llvm::Type::getInt64Ty(*LLVMContext);
            case AST_F32:
                return llvm::Type::getFloatTy(*LLVMContext);
            case AST_F64:
                return llvm::Type::getDoubleTy(*LLVMContext);
            case AST_U32:
                return llvm::Type::getInt64Ty(*LLVMContext);
            case AST_U64:
                return llvm::Type::getInt128Ty(*LLVMContext);
            case AST_BOOL:
                return llvm::Type::getInt1Ty(*LLVMContext);
            case AST_VOID:
                return llvm::Type::getVoidTy(*LLVMContext);
            case AST_CHAR:
                return llvm::Type::getInt8Ty(*LLVMContext);
            case AST_STRING:
                return llvm::ArrayType::get(llvm::Type::getInt8Ty(*LLVMContext), 0);

            default:
                LOG_ERROR_F("variable of type \"%d\" is currently not supported\n", type->type);
                exit(1);
        }

        return nullptr;
    }

    llvm::GlobalVariable* LLVMGenerator::generateGlobalVar(ASTGlobal_T* global)
    {
        llvm::GlobalVariable* llvmVar = new llvm::GlobalVariable(*llvmModule, generateType(global->type), false, llvm::GlobalValue::CommonLinkage, 0, global->name);

        return llvmVar;
    }

    llvm::Function* LLVMGenerator::generateFunction(ASTFunction_T* func)
    {
        llvm::Function* function = llvmModule->getFunction(func->name);
        if(!function)
            function = createFunction(func);
        if(!function)
            return nullptr;

        generateCompound(func->body, function);

        llvm::verifyFunction(*function);
        return function;
    }

    llvm::Function* LLVMGenerator::createFunction(ASTFunction_T* func)
    {
        std::vector<llvm::Type*> argumentTypes;
        for(size_t i = 0; i < func->args->size; i++)
        {
            ASTArgument_T* arg = (ASTArgument_T*) func->args->items[i];
            argumentTypes.insert(argumentTypes.end(), generateType(arg->dataType));
        }

        llvm::FunctionType* ft = llvm::FunctionType::get(generateType(func->returnType), argumentTypes, false);
        llvm::Function* f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, func->name, llvmModule.get());

        size_t idx = 0;
        for(auto &arg : f->args())
        {
            arg.setName(((ASTArgument_T*) func->args->items[idx++])->name);
        }

        return f;
    }

    llvm::BasicBlock* LLVMGenerator::generateCompound(ASTCompound_T* com, llvm::Function* funcRef, std::string name)
    {
        llvm::BasicBlock* bb = llvm::BasicBlock::Create(*LLVMContext, name, funcRef);
        llvmBuilder->SetInsertPoint(bb);

        namedValues.clear();
        for(auto &arg : funcRef->args())
            namedValues[std::string(arg.getName())] = &arg;

        for(size_t i = 0; i < com->stmts->size; i++)
        {
            ASTStmt_T* stmt = (ASTStmt_T*) com->stmts->items[i];

            switch(stmt->type)
            {
                case STMT_LET:
                    break;

                case STMT_LOOP:
                    break;

                case STMT_RETURN:
                    break;

                case STMT_IF:
                    break;

                case STMT_MATCH:
                    break;

                case STMT_EXPRESSION:
                    break;

                default:
                    break;
            }
        }

        return bb;
    }
}