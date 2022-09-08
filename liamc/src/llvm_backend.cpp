#include "llvm_backend.h"

LLVMBackend::LLVMBackend() {
    context = new llvm::LLVMContext();
    module  = std::make_unique<llvm::Module>("liam-llvm-module", *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);

    function_map = std::map<std::string, llvm::Function *>();
    struct_map = std::map<std::string, llvm::StructType *>();
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

    switch (statement->statement_type) {
        case StatementType::STATEMENT_FN:
            emit_fn_statement(static_cast<FnStatement *>(statement));
            break;
        case StatementType::STATEMENT_STRUCT:
            emit_struct_statement(static_cast<StructStatement *>(statement));
            break;
        case StatementType::STATEMENT_RETURN:
            emit_return_statement(static_cast<ReturnStatement *>(statement));
            break;
        case StatementType::STATEMENT_EXPRESSION:
            emit_expression_statement(static_cast<ExpressionStatement *>(statement));
            break;

        default:
            panic("Cannot emit this statement in llvm backend");
    }

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

    // converting each type in the function params to llvm types
    auto param_type_list = std::vector<llvm::Type *>();
    for (s32 i = 0; i < statement->params.size(); i++)
    { param_type_list.push_back(map_liam_type_to_llvm_type(context, statement->params[i].two->type_info)); }

    // function type decl
    llvm::FunctionType *ft = llvm::FunctionType::get(
        map_liam_type_to_llvm_type(context, statement->return_type->type_info), param_type_list, false
    );
    llvm::Function *func =
        llvm::Function::Create(ft, llvm::Function::ExternalLinkage, statement->identifier.string, module.get());

    // and set the name of each arg in the function to the correct name
    for (u32 i = 0; i < func->arg_size(); i++)
    { func->getArg(i)->setName(statement->params[i].one.string); }

    // add llvm function type to function map for use with call expressions
    function_map[statement->identifier.string] = func;

    // func body decl
    llvm::BasicBlock *block = llvm::BasicBlock::Create(*context, statement->identifier.string + "_body", func);
    builder->SetInsertPoint(block);
    emit_scope_statement(statement->body);

    llvm::verifyFunction(*func);
}

void LLVMBackend::emit_struct_statement(StructStatement *statement) {
    auto struct_type = llvm::StructType::create(*context, statement->identifier.string);

    std::vector<llvm::Type *> member_types = {};

    for(auto [name, type] : statement->members) {
        member_types.push_back(map_liam_type_to_llvm_type(context, type->type_info));
    }

    struct_type->setBody(member_types);

}

void LLVMBackend::emit_expression_statement(ExpressionStatement *statement) {
    emit_expression(statement->expression);
}

llvm::Value *LLVMBackend::emit_expression(Expression *expression) {

    switch (expression->type)
    {
    case ExpressionType::EXPRESSION_BOOL_LITERAL:
        return emit_bool_literal_expression(static_cast<BoolLiteralExpression *>(expression));
    case ExpressionType::EXPRESSION_NUMBER_LITERAL:
        return emit_int_literal_expression(static_cast<NumberLiteralExpression *>(expression));
    case ExpressionType::EXPRESSION_CALL:
        return emit_call_expression(static_cast<CallExpression *>(expression));
    default:
        panic("Cannot emit this expression yet in llvm backend...");
        return NULL;
    }
}

llvm::Value *LLVMBackend::emit_bool_literal_expression(BoolLiteralExpression *expression) {
    if (expression->value.string == "true")
    { return get_llvm_true_literal(context); }
    else
    { return get_llvm_false_literal(context); }
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
    auto func                        = function_map[identifier->identifier.string];

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