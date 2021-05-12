#include "llvm_generator.hpp"
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/PassManager.h>
#include <memory>
#include "../../io/log.h"
#include "llvm/Bitcode/BitcodeWriter.h"

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

/// putchard - putchar that takes a double and returns 0.
extern "C" DLLEXPORT double putchard(double X) {
  fputc((char)X, stderr);
  return 0;
}

/// printd - printf that takes a double prints it as "%f\n", returning 0.
extern "C" DLLEXPORT double printd(double X) {
  fprintf(stderr, "%f\n", X);
  return 0;
}

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

    int LLVMGenerator::emitToFile(std::string targetPath)
    {
        //TODO
        return 0;
    }

    LLVMGenerator::status LLVMGenerator::generate(ASTProgram_T *ast)
    {
        for(size_t i = 0; i < ast->files->size; i++)
            generateFile((ASTFile_T*) ast->files->items[i]);

        llvmModule->print(llvm::errs(), nullptr);
        return STATUS_OK;
    }

    void LLVMGenerator::generateFile(ASTFile_T *ast)
    {
        for(size_t i = 0; i < ast->globals->size; i++)
        {
            generateGlobalVar((ASTGlobal_T*) ast->globals->items[i]);
        }

        for(size_t i = 0; i < ast->functions->size; i++)
        {
            generateFunction((ASTFunction_T*) ast->functions->items[i]);
        }
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
            case AST_POINTER:
                return llvm::PointerType::get(generateType(type->subtype), 8);
            case AST_ARRAY:
                return llvm::ArrayType::get(generateType(type->subtype), 0);

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

                case STMT_RETURN:{
                    ASTReturn_T* ret = (ASTReturn_T*) stmt->stmt;

                    llvmBuilder->CreateRet(generateExpression(ret->value));
                } break;

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

    llvm::Value* LLVMGenerator::generateExpression(ASTExpr_T* expr)
    {
        switch(expr->type)
        {
            case EXPR_INT_LITERAL:
                return llvm::ConstantInt::get(*LLVMContext, llvm::APInt(32, ((ASTInt_T*) expr->expr)->_int, false));
            case EXPR_FLOAT_LITERAL:
                return llvm::ConstantFP::get(*LLVMContext, llvm::APFloat(((ASTFloat_T*) expr->expr)->_float));
            case EXPR_BOOL_LITERAL:
                return llvm::ConstantInt::get(*LLVMContext, llvm::APInt(1, ((ASTBool_T*) expr->expr)->_bool, false));
            case EXPR_CHAR_LITERAL:
                return llvm::ConstantInt::get(*LLVMContext, llvm::APInt(8, ((ASTChar_T*) expr->expr)->_char, false));
            default:  
                LOG_ERROR_F("Expressions of type %d are currently not supported", expr->type);
                exit(1);
        }
    }
}