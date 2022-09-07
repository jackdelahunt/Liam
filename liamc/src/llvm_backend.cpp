#include "llvm_backend.h"

LLVMBackend::LLVMBackend() {
    context = new llvm::LLVMContext();
    module  = std::make_unique<llvm::Module>("liam-llvm-module", *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);

    function_map = std::map<std::string, llvm::Function *>();
}

void LLVMBackend::emit(File *file) {
    for (auto stmt : file->statements)
    { emit_statement(stmt); }

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
    if (!Target)
    {
        llvm::errs() << Error;
        return;
    }

    auto CPU      = "generic";
    auto Features = "";

    llvm::TargetOptions opt;
    auto RM               = llvm::Optional<llvm::Reloc::Model>();
    auto TheTargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

    module->setDataLayout(TheTargetMachine->createDataLayout());

    auto Filename = "liam.o";
    std::error_code EC;
    llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);

    if (EC)
    {
        llvm::errs() << "Could not open file: " << EC.message();
        return;
    }

    llvm::legacy::PassManager pass;
    auto FileType = llvm::CGFT_ObjectFile;

    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType))
    {
        llvm::errs() << "TheTargetMachine can't emit a file of this type";
        return;
    }

    pass.run(*module);
    dest.flush();

    llvm::outs() << "Wrote " << Filename << "\n";
}

void LLVMBackend::emit_statement(Statement *statement) {

    if (statement->statement_type == StatementType::STATEMENT_FN)
    {
        emit_fn_statement(static_cast<FnStatement *>(statement));
        return;
    }

    if (statement->statement_type == StatementType::STATEMENT_RETURN)
    {
        emit_return_statement(static_cast<ReturnStatement *>(statement));
        return;
    }

    panic("Cannot emit this statement in llvm backend");
}

void LLVMBackend::emit_return_statement(ReturnStatement *statement) {
    if (statement->expression == NULL)
        return;

    auto ret = emit_expression(statement->expression);
    builder->CreateRet(ret);
}

void LLVMBackend::emit_scope_statement(ScopeStatement *statement) {
    for (auto stmt : statement->statements)
    { emit_statement(stmt); }
}

void LLVMBackend::emit_fn_statement(FnStatement *statement) {
    // function type decl
    llvm::FunctionType *ft =
        llvm::FunctionType::get(map_liam_type_to_llvm_type(context, statement->return_type->type_info), false);
    llvm::Function *func =
        llvm::Function::Create(ft, llvm::Function::ExternalLinkage, statement->identifier.string, module.get());

    // add llvm function type to function map for use with call expressions
    function_map[statement->identifier.string] = func;

    // func body decl
    llvm::BasicBlock *block = llvm::BasicBlock::Create(*context, statement->identifier.string + "_body", func);
    builder->SetInsertPoint(block);
    emit_scope_statement(statement->body);

    llvm::verifyFunction(*func);
}

void LLVMBackend::emit_if_statement(IfStatement *statement) {

    // https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl05.html#llvm-ir-for-if-then-else

    llvm::Value * condition = emit_expression(statement->expression);

    // compare expression to see if it is not equal to zero
    llvm::Value *compare_expression = builder->CreateFCmpONE(condition, get_llvm_false_literal(context), "if_condition");

    // merge block -> the rest of the code after this if statement
    llvm::Function *parent_func = builder->GetInsertBlock()->getParent();
    llvm::BasicBlock *if_block = llvm::BasicBlock::Create(*context, "if_block", parent_func);
    llvm::BasicBlock *else_block = llvm::BasicBlock::Create(*context, "else_block");
    llvm::BasicBlock *merge_block = llvm::BasicBlock::Create(*context, "merge");

    // conditional branch id the condition is true then jump to if_block and
    // if it is not true move to else block
    builder->CreateCondBr(condition, if_block, else_block);

    // emit the scope statement into the if block
    // then at the end of the if block create a branch to the merge block
    builder->SetInsertPoint(if_block);
    emit_scope_statement(statement->body);
    builder->CreateBr(merge_block);

    // after emitting the scope statement in the if statement
    // the merge may be change so just use that
    merge_block = builder->GetInsertBlock();

    // else block was not added to build when it was created so add it now
    parent_func->getBasicBlockList().push_back(else_block);
    builder->SetInsertPoint(else_block);
    emit_else_statement(statement->else_statement);
    builder->CreateBr(merge_block);

    // codegen of 'Else' can change the current block, update ElseBB for the PHI.
    else_block = builder->GetInsertBlock();
}

llvm::Value *LLVMBackend::emit_expression(Expression *expression) {

    switch (expression->type) {
        case ExpressionType::EXPRESSION_BOOL_LITERAL:
            return emit_bool_literal_expression(static_cast<BoolLiteralExpression *>(expression));
        case ExpressionType::EXPRESSION_NUMBER_LITERAL:
            return emit_int_literal_expression(static_cast<NumberLiteralExpression *>(expression));
        case  ExpressionType::EXPRESSION_CALL:
            return emit_call_expression(static_cast<CallExpression *>(expression));
        default:
            panic("Cannot emit this expression yet in llvm backend...");
        return NULL;
    }
}

llvm::Value *LLVMBackend::emit_bool_literal_expression(BoolLiteralExpression *expression) {
    if(expression->value.string == "true") {
        return get_llvm_true_literal(context);
    } else {
        return get_llvm_false_literal(context);
    }
}

llvm::Value *LLVMBackend::emit_int_literal_expression(NumberLiteralExpression *expression) {
    auto number_type_info = static_cast<NumberTypeInfo *>(expression->type_info);

    switch (number_type_info->type)
    {
    case NumberType::SIGNED:
        return llvm::ConstantInt::get(*context, llvm::APInt(number_type_info->size, expression->number, true));
    case NumberType::UNSIGNED:
        return llvm::ConstantInt::get(*context, llvm::APInt(number_type_info->size, expression->number, false));
    case NumberType::FLOAT:
        return llvm::ConstantFP::get(*context, llvm::APFloat((float)expression->number));
    }
}

llvm::Value *LLVMBackend::emit_call_expression(CallExpression *expression) {
    IdentifierExpression *identifier = static_cast<IdentifierExpression *>(expression->identifier);
    auto func = function_map[identifier->identifier.string];

    return builder->CreateCall(func, {}, identifier->identifier.string + "_call");
}

llvm::Type *map_liam_type_to_llvm_type(llvm::LLVMContext *context, TypeInfo *type_info) {
    switch (type_info->type)
    {
    case TypeInfoType::VOID:
        return llvm::Type::getVoidTy(*context);
    case TypeInfoType::NUMBER: {
        auto number_type_info = static_cast<NumberTypeInfo *>(type_info);

        switch (number_type_info->type)
        {
        case NumberType::SIGNED:
        case NumberType::UNSIGNED:
            // TODO: does not seem to be a unsigned int llvm type maybe this will
            // cause issues in code gen
            return llvm::Type::getIntNTy(*context, number_type_info->size);
        case NumberType::FLOAT:
            return llvm::Type::getFloatTy(*context);
        }
    }
    case TypeInfoType::BOOLEAN:
        return llvm::Type::getInt1Ty(*context);
    default:
        panic("Cannot convert this type to llvm type yet... :[");
    }
}

llvm::Value *get_llvm_true_literal(llvm::LLVMContext *context) {
    return llvm::ConstantInt::get(*context, llvm::APInt(1, 1));
}
llvm::Value *get_llvm_false_literal(llvm::LLVMContext *context) {
    return llvm::ConstantInt::get(*context, llvm::APInt(1, 0));
}