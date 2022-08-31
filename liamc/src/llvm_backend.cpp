#include "llvm_backend.h"


LLVMBackend::LLVMBackend() {
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>("liam-llvm-module", *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);
}

void LLVMBackend::emit(File *file) {
    for(auto stmt : file->statements) {
        emit_statement(stmt);
    }

    module->print(llvm::outs(), nullptr);

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(TargetTriple);

    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!Target) {
        llvm::errs() << Error;
        return;
    }

    auto CPU = "generic";
    auto Features = "";

    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    auto TheTargetMachine =
            Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

    module->setDataLayout(TheTargetMachine->createDataLayout());

    auto Filename = "liam.o";
    std::error_code EC;
    llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);

    if (EC) {
        llvm::errs() << "Could not open file: " << EC.message();
        return;
    }

    llvm::legacy::PassManager pass;
    auto FileType = llvm::CGFT_ObjectFile;

    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        llvm::errs() << "TheTargetMachine can't emit a file of this type";
        return;
    }

    pass.run(*module);
    dest.flush();

    llvm::outs() << "Wrote " << Filename << "\n";
}

void LLVMBackend::emit_statement(Statement *statement) {

    if(statement->statement_type == StatementType::STATEMENT_FN) {
        emit_fn_statement(static_cast<FnStatement *>(statement));
        return;
    }

    if(statement->statement_type == StatementType::STATEMENT_RETURN) {
        emit_return_statement(static_cast<ReturnStatement *>(statement));
        return;
    }

    panic("Uh oh");
}

void LLVMBackend::emit_return_statement(ReturnStatement *statement) {
    if(statement->expression == NULL) return;

    auto ret = emit_expression(statement->expression);
    builder->CreateRet(ret);
}

void LLVMBackend::emit_scope_statement(ScopeStatement *statement) {
   for(auto stmt : statement->statements) {
       emit_statement(stmt);
   }
}

void LLVMBackend::emit_fn_statement(FnStatement *statement) {
    // function type decl
    llvm::FunctionType *ft = llvm::FunctionType::get(llvm::Type::getInt64Ty(*context), false);
    llvm::Function *func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, statement->identifier.string, module.get());

    // func body decl
    llvm::BasicBlock *block = llvm::BasicBlock::Create(*context, statement->identifier.string + "_body", func);
    builder->SetInsertPoint(block);
    emit_scope_statement(statement->body);

    llvm::verifyFunction(*func);
}

llvm::Value *LLVMBackend::emit_expression(Expression *expression) {
    if(expression->type == ExpressionType::EXPRESSION_NUMBER_LITERAL) {
        return emit_int_literal_expression(static_cast<NumberLiteralExpression *>(expression));
    }

    return NULL;
}

llvm::Value *LLVMBackend::emit_int_literal_expression(NumberLiteralExpression *expression) {
    auto number_type_info = static_cast<NumberTypeInfo *>(expression->type_info);

    if(number_type_info->type != NumberType::SIGNED) panic("Can only emit ints right now :[");
    if(number_type_info->size != 64) panic("Can only emit 64 bit numbers right now :[");

    return llvm::ConstantInt::get(*context, llvm::APInt(64, (int)expression->number, true));
}
