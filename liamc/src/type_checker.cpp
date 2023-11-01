#include "type_checker.h"

#include <assert.h>
#include <format>
#include <tuple>

#include "args.h"
#include "errors.h"
#include "liam.h"
#include "parser.h"
#include "utils.h"

SymbolTable::SymbolTable(CompilationUnit *current_file) {
    this->compilation_unit         = current_file;
    this->local_generic_type_table = std::unordered_map<std::string, TypeInfo *>();
    this->identifier_table         = std::unordered_map<std::string, TypeInfo *>();
}

void SymbolTable::add_identifier_type(std::string identifier, TypeInfo *type_info) {
    if (identifier_table.count(identifier) > 0)
    {
        TypeCheckerError::make(this->compilation_unit->file_data->path.string())
            // TODO: figure out how to replace this
            //            .add_related_token(identifier)
            .set_message("Duplicate creation of identifier \"" + identifier + "\"")
            .report();

        return;
    }

    identifier_table[identifier] = type_info;
}

std::tuple<TypeInfo *, bool> SymbolTable::get_identifier_type(std::string identifier) {
    if (identifier_table.count(identifier) > 0)
    { return {this->identifier_table[identifier], false}; }

    return {NULL, true};
}

void SymbolTable::add_compiler_generated_identifier(std::string identifier, TypeInfo *type_info) {
    ASSERT(identifier_table.count(identifier) == 0);
    identifier_table[identifier] = type_info;
}

SymbolTable SymbolTable::copy() {
    SymbolTable st = *this;
    return st;
}

TypeChecker::TypeChecker() {
    compilation_unit = NULL;

    this->scopes =     std::list<std::unordered_map<std::string, TypeInfo *>>();

    this->builtin_type_table   = std::unordered_map<std::string, TypeInfo *>();
    this->top_level_type_table = std::unordered_map<std::string, TopLevelDescriptor>();
    this->top_level_type_table = std::unordered_map<std::string, TopLevelDescriptor>();

    this->builtin_type_table["void"] = new VoidTypeInfo();
    this->builtin_type_table["bool"] = new BoolTypeInfo();
    this->builtin_type_table["str"]  = new StrTypeInfo();
    this->builtin_type_table["u8"]   = new NumberTypeInfo(8, NumberType::UNSIGNED);
    this->builtin_type_table["i8"]   = new NumberTypeInfo(8, NumberType::SIGNED);
    this->builtin_type_table["u16"]  = new NumberTypeInfo(16, NumberType::UNSIGNED);
    this->builtin_type_table["i16"]  = new NumberTypeInfo(16, NumberType::SIGNED);
    this->builtin_type_table["u32"]  = new NumberTypeInfo(32, NumberType::UNSIGNED);
    this->builtin_type_table["i32"]  = new NumberTypeInfo(32, NumberType::SIGNED);
    this->builtin_type_table["f32"]  = new NumberTypeInfo(32, NumberType::FLOAT);
    this->builtin_type_table["u64"]  = new NumberTypeInfo(64, NumberType::UNSIGNED);
    this->builtin_type_table["i64"]  = new NumberTypeInfo(64, NumberType::SIGNED);
    this->builtin_type_table["f64"]  = new NumberTypeInfo(64, NumberType::FLOAT);
}

void TypeChecker::new_scope() {
    this->scopes.emplace_back();
}

void TypeChecker::delete_scope() {
    ASSERT_MSG(this->scopes.size() > 0, "Must be an active scope to add to");
    this->scopes.pop_front();
}

void TypeChecker::add_to_scope(TokenIndex token_index, TypeInfo *type_info) {
    ASSERT_MSG(this->scopes.size() > 0, "Must be an active scope to add to");
    std::string identifier = this->compilation_unit->get_token_string_from_index(token_index);
    this->scopes.front()[identifier] = type_info;
}

TypeInfo *TypeChecker::get_from_scope(TokenIndex token_index) {
    ASSERT_MSG(this->scopes.size() > 0, "Must be an active scope to add to");
    std::string identifier = this->compilation_unit->get_token_string_from_index(token_index);
    return this->scopes.front()[identifier];
}

void TypeChecker::add_type(CompilationUnit *file, TokenIndex identifier, TypeInfo *type_info) {
    ASSERT(type_info->type == TypeInfoType::STRUCT);

    std::string identifier_string = file->get_token_string_from_index(identifier);

    if (this->top_level_type_table.count(identifier_string) > 0)
    { panic("Duplicate creation of type: " + identifier_string); }

    TopLevelDescriptor descriptor = TopLevelDescriptor{
        .identifier = identifier_string,
        .type_info  = type_info,
    };

    this->top_level_type_table[identifier_string] = descriptor;
}

void TypeChecker::add_function(CompilationUnit *file, TokenIndex token, TypeInfo *type_info) {
    std::string token_as_string = this->compilation_unit->get_token_string_from_index(token);
    if (this->top_level_function_table.count(token_as_string) > 0)
    { panic("Duplicate creation of function: " + token_as_string); }

    TopLevelDescriptor descriptor = TopLevelDescriptor{
        .identifier = token_as_string,
        .type_info  = type_info,
    };

    this->top_level_function_table[token_as_string] = descriptor;
}

std::tuple<TypeInfo *, bool> TypeChecker::get_type(std::string identifier) {
    if (this->top_level_type_table.count(identifier) > 0)
    { return {this->top_level_type_table[identifier].type_info, false}; }

    if (this->builtin_type_table.count(identifier) > 0)
    { return {this->builtin_type_table[identifier], false}; }

    return {nullptr, true};
}

std::tuple<TypeInfo *, bool> TypeChecker::get_function(std::string identifier) {
    if (this->top_level_function_table.count(identifier) > 0)
    { return {this->top_level_function_table[identifier].type_info, false}; }

    return {nullptr, true};
}

void TypeChecker::type_check(CompilationUnit *file) {

    this->compilation_unit = file;

    // add symbols for structs, enums and aliass
    for (auto stmt : this->compilation_unit->top_level_struct_statements)
    { TRY_CALL_VOID(type_check_struct_symbol(stmt)); }

    for (auto stmt : this->compilation_unit->top_level_fn_statements)
    { TRY_CALL_VOID(type_check_fn_symbol(stmt)); }

    for (auto stmt : this->compilation_unit->top_level_struct_statements)
    { TRY_CALL_VOID(type_check_struct_statement_full(stmt)); }

    for (auto stmt : this->compilation_unit->top_level_fn_statements)
    { TRY_CALL_VOID(type_check_fn_decl(stmt)); }

    // finally do the function body pass
    for (auto stmt : this->compilation_unit->top_level_fn_statements)
    { TRY_CALL_VOID(type_check_fn_statement_full(stmt)); }
}

StructTypeInfo *get_struct_type_info_from_type_info(TypeInfo *type_info) {
    StructTypeInfo *parent_type_info = NULL;

    if (type_info->type == TypeInfoType::STRUCT)
    { parent_type_info = (StructTypeInfo *)type_info; }
    else if (type_info->type == TypeInfoType::STRUCT_INSTANCE)
    {
        auto struct_instance_type = (StructInstanceTypeInfo *)(type_info);
        parent_type_info          = (StructTypeInfo *)struct_instance_type->struct_type;
    }

    return parent_type_info;
}

void TypeChecker::type_check_fn_symbol(FnStatement *statement) {
    this->add_function(this->compilation_unit, statement->identifier, new FnTypeInfo(NULL, NULL, {}));
}

void TypeChecker::type_check_struct_symbol(StructStatement *statement) {
    // this type does exist but the type info has not been type checked yet so just
    // add it to the table and leave its type info blank until we type check it

    std::string identifier_string = this->compilation_unit->get_token_string_from_index(statement->identifier);
    auto [type_info, failed]      = this->get_type(identifier_string);
    if (!failed)
    {

        TypeCheckerError::make(compilation_unit->file_data->path.string())
            // TODO: replace this somehow
            //            .add_related_token(statement->identifier)
            .set_message("Re-declaration of type \"" + identifier_string + "\"")
            .report();
        return;
    }

    this->add_type(this->compilation_unit, statement->identifier, new StructTypeInfo({}, {}));
}

void TypeChecker::type_check_fn_decl(FnStatement *statement) {
    SymbolTable symbol_table = SymbolTable(compilation_unit);

    auto param_type_infos = std::vector<TypeInfo *>();
    for (auto &[identifier, expr] : statement->params)
    {
        TRY_CALL_VOID(type_check_type_expression(expr, &symbol_table))
        param_type_infos.push_back(expr->type_info);
    }

    TRY_CALL_VOID(type_check_type_expression(statement->return_type, &symbol_table));

    // TODO remove this
    std::string name_as_string = this->compilation_unit->get_token_string_from_index(statement->identifier);

    // add this fn decl to parent symbol table
    auto current_type_info = (FnTypeInfo *)this->top_level_function_table[name_as_string].type_info;

    current_type_info->return_type = statement->return_type->type_info;
    current_type_info->args        = param_type_infos;
}

void TypeChecker::type_check_fn_statement_full(FnStatement *statement) {
    SymbolTable symbol_table = SymbolTable(compilation_unit);

    // getting the already existing type info for the function is done in 2 ways
    // 1. looking into the symbol table of the current scope (normal function)
    // 2. looking into the member functions of a type (member function)
    FnTypeInfo *fn_type_info = NULL;

        std::string name_as_string = this->compilation_unit->get_token_string_from_index(statement->identifier);
        fn_type_info               = (FnTypeInfo *)this->top_level_function_table[name_as_string].type_info;
        ASSERT(fn_type_info);

    // params and get type expressions
    auto args = std::vector<std::tuple<TokenIndex, TypeInfo *>>();
    args.reserve(fn_type_info->args.size());
    args.resize(fn_type_info->args.size());
    for (int i = 0; i < fn_type_info->args.size(); i++)
    {
        auto &[identifier, expr] = statement->params.at(i);
        args[i]                  = {identifier, fn_type_info->args.at(i)};
    }

    for (auto &[identifier, type_info] : args)
    {
        std::string identifier_string = this->compilation_unit->get_token_string_from_index(identifier);
        TRY_CALL_VOID(symbol_table.add_identifier_type(identifier_string, type_info));
    }

    // type statements and check return exists if needed
    this->new_scope();
    TRY_CALL_VOID(type_check_scope_statement(statement->body, &symbol_table, false));
    this->delete_scope();
    for (auto stmt : statement->body->statements)
    {
        if (stmt->statement_type == StatementType::STATEMENT_RETURN)
        {
            auto rt = static_cast<ReturnStatement *>(stmt);

            if (fn_type_info->return_type->type == TypeInfoType::VOID)
            {
                if (rt->expression != NULL)
                {
                    TypeCheckerError::make(compilation_unit->file_data->path.string())
                        .set_expr_1(rt->expression)
                        .set_message("found expression in return when return type is void")
                        .report();

                    return;
                }
            }
            else
            {
                if (!type_match(fn_type_info->return_type, rt->expression->type_info))
                {
                    ErrorReporter::report_type_checker_error(
                        compilation_unit->file_data->path.string(), rt->expression, NULL, statement->return_type, NULL,
                        "Mismatch types in function, return types do not match"
                    );
                    return;
                }
            }
        }
    }
}

void TypeChecker::type_check_struct_statement_full(StructStatement *statement) {

    SymbolTable symbol_table = SymbolTable(compilation_unit);

    auto members_type_info = std::vector<std::tuple<std::string, TypeInfo *>>();
    members_type_info.reserve(statement->members.size());
    members_type_info.resize(statement->members.size());
    for (i64 i = 0; i < statement->members.size(); i++)
    {
        auto [member, expr] = statement->members.at(i);
        TRY_CALL_VOID(type_check_type_expression(expr, &symbol_table));
        std::string member_string = this->compilation_unit->get_token_string_from_index(member);
        members_type_info[i]      = {member_string, expr->type_info};
    }

    std::string identifier_string = this->compilation_unit->get_token_string_from_index(statement->identifier);

    // this struct has not been fully typed until now just add to the type that already exists in the
    // type table
    StructTypeInfo *sti = (StructTypeInfo *)this->top_level_type_table[identifier_string].type_info;
    sti->members        = members_type_info;
}

void TypeChecker::type_check_statement(Statement *statement, SymbolTable *symbol_table) {
    switch (statement->statement_type)
    {
    case StatementType::STATEMENT_RETURN:
        return type_check_return_statement(dynamic_cast<ReturnStatement *>(statement), symbol_table);
    case StatementType::STATEMENT_BREAK:
        return type_check_break_statement(dynamic_cast<BreakStatement *>(statement), symbol_table);
    case StatementType::STATEMENT_ASSIGNMENT:
        return type_check_assigment_statement(dynamic_cast<AssigmentStatement *>(statement), symbol_table);
    case StatementType::STATEMENT_EXPRESSION:
        return type_check_expression_statement(dynamic_cast<ExpressionStatement *>(statement), symbol_table);
    case StatementType::STATEMENT_LET:
        return type_check_let_statement(dynamic_cast<LetStatement *>(statement), symbol_table);
    case StatementType::STATEMENT_FOR:
        return type_check_for_statement(dynamic_cast<ForStatement *>(statement), symbol_table);
    case StatementType::STATEMENT_IF:
        return type_check_if_statement(dynamic_cast<IfStatement *>(statement), symbol_table);
    case StatementType::STATEMENT_CONTINUE:
        break;
    case StatementType::STATEMENT_STRUCT:
    case StatementType::STATEMENT_IMPORT:
    case StatementType::STATEMENT_FN:
        ASSERT_MSG(0, "These should of already been type checked in the first pass");
    default:
        ASSERT_MSG(
            0, "Statement not implemented in type checker, id -> " + std::to_string((int)statement->statement_type)
        );
    }
}

void TypeChecker::type_check_return_statement(ReturnStatement *statement, SymbolTable *symbol_table) {
    if (statement->expression)
        TRY_CALL_VOID(type_check_expression(statement->expression, symbol_table));
}

void TypeChecker::type_check_break_statement(BreakStatement *statement, SymbolTable *symbol_table) {
}

void TypeChecker::type_check_let_statement(LetStatement *statement, SymbolTable *symbol_table) {
    TRY_CALL_VOID(type_check_expression(statement->rhs, symbol_table));

    // if let type is there type match both and set var type
    // to the let type... else just set it to the rhs
    if (statement->type)
    {
        TRY_CALL_VOID(type_check_type_expression(statement->type, symbol_table));
        if (!type_match(statement->type->type_info, statement->rhs->type_info))
        {
            ErrorReporter::report_type_checker_error(
                compilation_unit->file_data->path.string(), statement->rhs, NULL, statement->type, NULL,
                "Mismatched types in let statement"
            );
            return;
        }
    }

    auto string = this->compilation_unit->get_token_string_from_index(statement->identifier);
    TRY_CALL_VOID(symbol_table->add_identifier_type(string, statement->rhs->type_info));

    this->add_to_scope(statement->identifier, statement->rhs->type_info);
}

void TypeChecker::type_check_scope_statement(
    ScopeStatement *statement, SymbolTable *symbol_table, bool copy_symbol_table
) {
    // this is kind of a mess... oh jeez
    SymbolTable *scopes_symbol_table = symbol_table;
    SymbolTable possible_copy;
    if (copy_symbol_table)
    {
        possible_copy       = scopes_symbol_table->copy();
        scopes_symbol_table = &possible_copy;
    }

    for (auto stmt : statement->statements)
    { TRY_CALL_VOID(type_check_statement(stmt, scopes_symbol_table)); }
}

void TypeChecker::type_check_for_statement(ForStatement *statement, SymbolTable *symbol_table) {
    auto table_copy = *symbol_table;
    TRY_CALL_VOID(type_check_statement(statement->assign, &table_copy));
    TRY_CALL_VOID(type_check_expression(statement->condition, &table_copy));
    TRY_CALL_VOID(type_check_statement(statement->update, &table_copy));

    if (statement->condition->type_info->type != TypeInfoType::BOOLEAN)
    { panic("Second statement in for loop needs to evaluate to a bool"); }

    TRY_CALL_VOID(type_check_scope_statement(statement->body, &table_copy, false));
}

void TypeChecker::type_check_if_statement(IfStatement *statement, SymbolTable *symbol_table) {
    // if may declare new varaibles that we do not want to leak into outher
    // scope like with the is expression
    auto copy = symbol_table->copy();

    TRY_CALL_VOID(type_check_expression(statement->expression, &copy));

    if (!type_match(statement->expression->type_info, this->builtin_type_table["bool"]))
    { panic("If statement must be passed a bool"); }

    TRY_CALL_VOID(type_check_scope_statement(statement->body, &copy));

    if (statement->else_statement)
    {
        // might not have else statement
        // not using symbol table copy either as symbols
        // should not leak to sub statements
        TRY_CALL_VOID(type_check_else_statement(statement->else_statement, symbol_table));
    }
}

void TypeChecker::type_check_else_statement(ElseStatement *statement, SymbolTable *symbol_table) {
    if (statement->if_statement)
    { TRY_CALL_VOID(type_check_if_statement(statement->if_statement, symbol_table)); }

    if (statement->body)
    { TRY_CALL_VOID(type_check_scope_statement(statement->body, symbol_table)); }
}

void TypeChecker::type_check_assigment_statement(AssigmentStatement *statement, SymbolTable *symbol_table) {
    TRY_CALL_VOID(type_check_expression(statement->lhs, symbol_table));
    TRY_CALL_VOID(type_check_expression(statement->assigned_to->expression, symbol_table));

    if (!type_match(statement->lhs->type_info, statement->assigned_to->expression->type_info))
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->path.string(), statement->lhs, statement->assigned_to->expression, NULL, NULL,
            "Type mismatch, trying to assign a identifier to an expression of different type"
        );
        return;
    }
}

void TypeChecker::type_check_expression_statement(ExpressionStatement *statement, SymbolTable *symbol_table) {
    TRY_CALL_VOID(type_check_expression(statement->expression, symbol_table));
}

void TypeChecker::type_check_expression(Expression *expression, SymbolTable *symbol_table) {
    switch (expression->type)
    {
    case ExpressionType::EXPRESSION_STRING_LITERAL:
        return type_check_string_literal_expression(dynamic_cast<StringLiteralExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_NUMBER_LITERAL:
        return type_check_number_literal_expression(dynamic_cast<NumberLiteralExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_BOOL_LITERAL:
        return type_check_bool_literal_expression(dynamic_cast<BoolLiteralExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_CALL:
        return type_check_call_expression(dynamic_cast<CallExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_IDENTIFIER:
        return type_check_identifier_expression(dynamic_cast<IdentifierExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_BINARY:
        return type_check_binary_expression(dynamic_cast<BinaryExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_UNARY:
        return type_check_unary_expression(dynamic_cast<UnaryExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_GET:
        return type_check_get_expression(dynamic_cast<GetExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_GROUP:
        return type_check_group_expression(dynamic_cast<GroupExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_NULL_LITERAL:
        return type_check_null_literal_expression(dynamic_cast<NullLiteralExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_ZERO_LITERAL:
        return type_check_zero_literal_expression(dynamic_cast<ZeroLiteralExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_INSTANTIATION:
        return type_check_instantiate_expression(dynamic_cast<InstantiateExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_STRUCT_INSTANCE:
        return type_check_struct_instance_expression(
            dynamic_cast<StructInstanceExpression *>(expression), symbol_table
        );
        break;
    default:
        panic("Expression not implemented in type checker");
    }
}

void TypeChecker::type_check_binary_expression(BinaryExpression *expression, SymbolTable *symbol_table) {
    TRY_CALL_VOID(type_check_expression(expression->left, symbol_table));
    TRY_CALL_VOID(type_check_expression(expression->right, symbol_table));

    if (!type_match(expression->left->type_info, expression->right->type_info))
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->path.string(), expression->left, expression->right, NULL, NULL,
            "Type mismatch in binary expression"
        );
        return;
    }

    TypeInfo *info = NULL;

    // logical ops - bools -> bool
    if (expression->op == TokenType::TOKEN_AND || expression->op == TokenType::TOKEN_OR)
    {
        if (expression->left->type_info->type != TypeInfoType::BOOLEAN &&
            expression->right->type_info->type != TypeInfoType::BOOLEAN)
        {
            ErrorReporter::report_type_checker_error(
                compilation_unit->file_data->path.string(), expression->left, expression->right, NULL, NULL,
                "Cannot use logical operators on non bool type"
            );
            return;
        }

        info = this->builtin_type_table["bool"];
    }

    // math ops - numbers -> numbers
    if (expression->op == TokenType::TOKEN_PLUS || expression->op == TokenType::TOKEN_STAR ||
        expression->op == TokenType::TOKEN_SLASH || expression->op == TokenType::TOKEN_MOD ||
        expression->op == TokenType::TOKEN_MINUS)
    {
        if (expression->left->type_info->type != TypeInfoType::NUMBER)
        {
            ErrorReporter::report_type_checker_error(
                compilation_unit->file_data->path.string(), expression->left, expression->right, NULL, NULL,
                "Cannot use arithmatic operator on non number"
            );
            return;
        }
        info = expression->left->type_info;
    }

    // math ops - numbers -> bool
    if (expression->op == TokenType::TOKEN_LESS || expression->op == TokenType::TOKEN_GREATER ||
        expression->op == TokenType::TOKEN_GREATER_EQUAL || expression->op == TokenType::TOKEN_LESS_EQUAL)
    {
        if (expression->left->type_info->type != TypeInfoType::NUMBER)
        {
            ErrorReporter::report_type_checker_error(
                compilation_unit->file_data->path.string(), expression->left, expression->right, NULL, NULL,
                "Cannot use comparison operator on non number"
            );
            return;
        }
        info = this->builtin_type_table["bool"];
    }

    // compare - any -> bool
    if (expression->op == TokenType::TOKEN_EQUAL || expression->op == TokenType::TOKEN_NOT_EQUAL)
    { info = this->builtin_type_table["bool"]; }

    assert(info != NULL);

    expression->type_info = info;
}

void TypeChecker::type_check_string_literal_expression(StringLiteralExpression *expression, SymbolTable *symbol_table) {
    expression->type_info = this->builtin_type_table["str"];
}

void TypeChecker::type_check_number_literal_expression(NumberLiteralExpression *expression, SymbolTable *symbol_table) {

    std::string token_string = this->compilation_unit->get_token_string_from_index(expression->token);

    auto [number, type, size] = extract_number_literal_size(token_string);

    if (size == -1)
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->path.string(), expression, NULL, NULL, NULL, "Problem parsing number literal"
        );
        return;
    }

    if (type != NumberType::FLOAT && number != (i64)number)
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->path.string(), expression, NULL, NULL, NULL,
            "Cannot use decimal point on non float types"
        );
        return;
    }

    expression->number = number;

    switch (type)
    {
    case NumberType::UNSIGNED: {

        if (number < 0)
        {
            ErrorReporter::report_type_checker_error(
                compilation_unit->file_data->path.string(), expression, NULL, NULL, NULL,
                "Unsigned number cannot be negative"
            );
            return;
        }

        if (size == 8)
        { expression->type_info = this->builtin_type_table["u8"]; }
        else if (size == 16)
        { expression->type_info = this->builtin_type_table["u16"]; }
        else if (size == 32)
        { expression->type_info = this->builtin_type_table["u32"]; }
        else if (size == 64)
        { expression->type_info = this->builtin_type_table["u64"]; }
    }
    break;
    case NumberType::SIGNED: {

        if (size == 8)
        { expression->type_info = this->builtin_type_table["i8"]; }
        else if (size == 16)
        { expression->type_info = this->builtin_type_table["i16"]; }
        else if (size == 32)
        { expression->type_info = this->builtin_type_table["i32"]; }
        else if (size == 64)
        { expression->type_info = this->builtin_type_table["i64"]; }
    }
    break;
    case NumberType::FLOAT: {
        if (size == 8 || size == 16)
        {
            ErrorReporter::report_type_checker_error(
                compilation_unit->file_data->path.string(), expression, NULL, NULL, NULL,
                "Cannot create float of this size can only use 32 and 64 sizes"
            );
            return;
        }
        else if (size == 32)
        { expression->type_info = this->builtin_type_table["f32"]; }
        else if (size == 64)
        { expression->type_info = this->builtin_type_table["f64"]; }
    }
    break;
    }
}

void TypeChecker::type_check_bool_literal_expression(BoolLiteralExpression *expression, SymbolTable *symbol_table) {
    expression->type_info = new BoolTypeInfo();
}

void TypeChecker::type_check_unary_expression(UnaryExpression *expression, SymbolTable *symbol_table) {
    TRY_CALL_VOID(type_check_expression(expression->expression, symbol_table));

    if (expression->op == TokenType::TOKEN_AMPERSAND)
    {
        expression->type_info = new PointerTypeInfo(expression->expression->type_info);
        return;
    }

    if (expression->op == TokenType::TOKEN_STAR)
    {
        if (expression->expression->type_info->type != TypeInfoType::POINTER)
        {
            ErrorReporter::report_type_checker_error(
                compilation_unit->file_data->path.string(), expression, NULL, NULL, NULL,
                "Cannot dereference non-pointer value"
            );
            return;
        }

        expression->type_info = ((PointerTypeInfo *)expression->expression->type_info)->to;
        return;
    }

    if (expression->op == TokenType::TOKEN_NOT)
    {
        if (expression->expression->type_info->type != TypeInfoType::BOOLEAN)
        {
            ErrorReporter::report_type_checker_error(
                compilation_unit->file_data->path.string(), expression, NULL, NULL, NULL,
                "Cannot use unary operator ! on non-boolean type"
            );
            return;
        }

        expression->type_info = expression->expression->type_info;
        return;
    }

    panic("Internal :: Unrecognised unary operator");
}

void TypeChecker::type_check_call_expression(CallExpression *expression, SymbolTable *symbol_table) {

    TRY_CALL_VOID(type_check_expression(expression->callee, symbol_table));
    auto callee_expression = expression->callee;

    if (expression->callee->type_info->type == TypeInfoType::FN)
    {
        TRY_CALL_VOID(type_check_fn_call_expression(expression, symbol_table));
        return;
    }

    ErrorReporter::report_type_checker_error(
        compilation_unit->file_data->path.string(), callee_expression, NULL, NULL, NULL, "Can only call functions"
    );
}

void TypeChecker::type_check_fn_call_expression(CallExpression *expression, SymbolTable *symbol_table) {

    auto callee_expression = expression->callee;

    auto arg_type_infos = std::vector<TypeInfo *>();
    for (auto arg : expression->args)
    {
        TRY_CALL_VOID(type_check_expression(arg, symbol_table));
        arg_type_infos.push_back(arg->type_info);
    }

    auto fn_type_info = static_cast<FnTypeInfo *>(callee_expression->type_info);
    if (fn_type_info->args.size() != arg_type_infos.size())
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->path.string(), callee_expression, NULL, NULL, NULL,
            std::format(
                "Incorrect number of arguments in call expression, expected {} got {}", fn_type_info->args.size(),
                arg_type_infos.size()
            )
        );
        return;
    }

    for (i32 i = 0; i < fn_type_info->args.size(); i++)
    {
        if (!type_match(fn_type_info->args.at(i), arg_type_infos.at(i)))
        {
            ErrorReporter::report_type_checker_error(
                compilation_unit->file_data->path.string(), callee_expression, expression->args.at(i), NULL, NULL,
                "Mismatched types function call"
            );
            return;
        }
    }

    expression->type_info = fn_type_info->return_type;
}

void TypeChecker::type_check_identifier_expression(IdentifierExpression *expression, SymbolTable *symbol_table) {
    std::string identifier_string = this->compilation_unit->get_token_string_from_index(expression->identifier);
    auto [type_info, failed]      = symbol_table->get_identifier_type(identifier_string);

    if (failed)
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->path.string(), expression, NULL, NULL, NULL,
            std::format("Unrecognized identifier \"{}\"", identifier_string)
        );
        return;
    }

    TypeInfo *new_type = this->get_from_scope(expression->identifier);

    expression->type_info = new_type;
}

void TypeChecker::type_check_get_expression(GetExpression *expression, SymbolTable *symbol_table) {
    TRY_CALL_VOID(type_check_expression(expression->lhs, symbol_table));

    TypeInfo *member_type_info                        = NULL; // populated every time
    StructTypeInfo *struct_type_info                  = NULL; // populated every time
    StructInstanceTypeInfo *struct_instance_type_info = NULL; // populated when the lhs is a struct instance
    if (expression->lhs->type_info->type == TypeInfoType::POINTER)
    {
        auto ptr_type_info = static_cast<PointerTypeInfo *>(expression->lhs->type_info);

        if (ptr_type_info->to->type == TypeInfoType::STRUCT)
        { struct_type_info = (StructTypeInfo *)ptr_type_info->to; }
        else if (ptr_type_info->to->type == TypeInfoType::STRUCT_INSTANCE)
        {
            struct_instance_type_info = (StructInstanceTypeInfo *)(ptr_type_info->to);
            struct_type_info          = (StructTypeInfo *)struct_instance_type_info->struct_type;
        }
        else
        {
            ErrorReporter::report_type_checker_error(
                compilation_unit->file_data->path.string(), expression->lhs, NULL, NULL, NULL,
                "Cannot derive member from non struct type"
            );
            return;
        }
    }
    else if (expression->lhs->type_info->type == TypeInfoType::STRUCT)
    { struct_type_info = static_cast<StructTypeInfo *>(expression->lhs->type_info); }
    else if (expression->lhs->type_info->type == TypeInfoType::STRUCT_INSTANCE)
    {
        struct_instance_type_info = static_cast<StructInstanceTypeInfo *>(expression->lhs->type_info);
        ASSERT(struct_instance_type_info->struct_type->type == TypeInfoType::STRUCT);
        struct_type_info = static_cast<StructTypeInfo *>(struct_instance_type_info->struct_type);
    }
    else
    {
        ErrorReporter::report_type_checker_error(
            this->compilation_unit->file_data->path, expression->lhs, NULL, NULL, NULL,
            "Cannot derive member from non struct type"
        );
        return;
    }

    // find the type info of the member we are looking for in the struct
    // definition, here it could be a base type or it could be a generic type
    // we are also checking if it is a member or a function member
    std::string member_string = this->compilation_unit->get_token_string_from_index(expression->member);

    for (auto [identifier, member_type] : struct_type_info->members)
    {
        if (identifier == member_string)
        {
            member_type_info = member_type;
            break;
        }
    }

    // identifier does not refer to a member in the struct type so check the function members
    if (member_type_info == NULL)
    {
        for (auto [identifier, function_member_type] : struct_type_info->member_functions)
        {
            if (identifier == member_string)
            {
                member_type_info = function_member_type;
                break;
            }
        }
    }

    if (member_type_info == NULL)
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->path.string(), expression, NULL, NULL, NULL,
            std::format("Cannot find member \"{}\" in struct", member_string)
        );
        return;
    }

    expression->type_info = member_type_info;
}

void TypeChecker::type_check_group_expression(GroupExpression *expression, SymbolTable *symbol_table) {
    TRY_CALL_VOID(type_check_expression(expression->expression, symbol_table));
    expression->type_info = expression->expression->type_info;
}

void TypeChecker::type_check_null_literal_expression(NullLiteralExpression *expression, SymbolTable *symbol_table) {
    expression->type_info = new PointerTypeInfo(new AnyTypeInfo());
}

void TypeChecker::type_check_zero_literal_expression(ZeroLiteralExpression *expression, SymbolTable *symbol_table) {
    expression->type_info = new AnyTypeInfo{TypeInfoType::ANY};
}

void TypeChecker::type_check_instantiate_expression(InstantiateExpression *expression, SymbolTable *symbol_table) {

    if (expression->expression->type != ExpressionType::EXPRESSION_STRUCT_INSTANCE)
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->path.string(), expression, expression->expression, NULL, NULL,
            "Cannot instantiate non-struct type"
        );
        return;
    }

    TRY_CALL_VOID(type_check_expression(expression->expression, symbol_table));

    expression->type_info = expression->expression->type_info;
}

void TypeChecker::type_check_struct_instance_expression(
    StructInstanceExpression *expression, SymbolTable *symbol_table
) {
    std::string identifier_string = this->compilation_unit->get_token_string_from_index(expression->identifier);
    auto [type_info, failed]      = this->get_type(identifier_string);
    if (failed)
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->path.string(), expression, NULL, NULL, NULL,
            "Unrecognised type in new expression"
        );
        return;
    }

    // check it's a struct
    if (type_info->type != TypeInfoType::STRUCT)
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->path.string(), expression, NULL, NULL, NULL,
            "Can only use struct types in new expression"
        );
        return;
    }

    auto struct_type_info = (StructTypeInfo *)type_info;

    // collect members from new constructor
    auto calling_args_type_infos = std::vector<std::tuple<std::string, TypeInfo *>>();
    for (auto [name_token_index, expr] : expression->named_expressions)
    {
        std::string name = this->compilation_unit->get_token_string_from_index(name_token_index);
        TRY_CALL_VOID(type_check_expression(expr, symbol_table));
        calling_args_type_infos.emplace_back(name, expr->type_info);
    }

    // check counts
    if (struct_type_info->members.size() != calling_args_type_infos.size())
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->path.string(), expression, NULL, NULL, NULL,
            std::format(
                "Incorrect number of arguments in new expression, expected {} got {}", struct_type_info->members.size(),
                calling_args_type_infos.size()
            )
        );
        return;
    }

    // check types
    for (i32 i = 0; i < calling_args_type_infos.size(); i++)
    {
        // new expression member and type info
        auto [expression_member, expression_type] = calling_args_type_infos.at(i);

        // struct member and type info
        auto [member, type] = struct_type_info->members.at(i);

        if (expression_member != member)
        {
            auto [name, expr] = expression->named_expressions.at(i);
            ErrorReporter::report_type_checker_error(
                compilation_unit->file_data->path.string(), expr, NULL, NULL, NULL,
                "Incorrect name specifier in new expression"
            );
            return;
        }
    }

    expression->type_info = struct_type_info;
}

void TypeChecker::type_check_type_expression(TypeExpression *type_expression, SymbolTable *symbol_table) {
    switch (type_expression->type)
    {
    case TypeExpressionType::TYPE_IDENTIFIER:
        type_check_identifier_type_expression(dynamic_cast<IdentifierTypeExpression *>(type_expression), symbol_table);
        break;
    case TypeExpressionType::TYPE_UNARY:
        type_check_unary_type_expression(dynamic_cast<UnaryTypeExpression *>(type_expression), symbol_table);
        break;
    default:
        panic("Not implemented for type checker");
    }
}

void TypeChecker::type_check_unary_type_expression(UnaryTypeExpression *type_expression, SymbolTable *symbol_table) {
    if (type_expression->unary_type == UnaryType::POINTER)
    {
        TRY_CALL_VOID(type_check_type_expression(type_expression->type_expression, symbol_table));
        type_expression->type_info = new PointerTypeInfo(type_expression->type_expression->type_info);
        return;
    }

    panic("Internal :: Cannot type check this operator yet...");
}

void TypeChecker::type_check_identifier_type_expression(
    IdentifierTypeExpression *type_expression, SymbolTable *symbol_table
) {
    std::string identifier_string = this->compilation_unit->get_token_string_from_index(type_expression->identifier);
    auto [type, error]            = this->get_type(identifier_string);
    if (error)
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->path.string(), NULL, NULL, type_expression, NULL,
            "Unrecognised type in type expression"
        );
        return;
    }

    type_expression->type_info = type;
}

bool type_match(TypeInfo *a, TypeInfo *b) {

    ASSERT_MSG(!(a->type == TypeInfoType::ANY && b->type == TypeInfoType::ANY), "Cannot compare 2 any types");

    if (a->type != b->type)
    {
        if (a->type != TypeInfoType::ANY && b->type != TypeInfoType::ANY)
        { return false; }
    }

    if (a->type == TypeInfoType::ANY)
        return true;

    if (b->type == TypeInfoType::ANY)
        return true;

    if (a->type == TypeInfoType::VOID || a->type == TypeInfoType::BOOLEAN || a->type == TypeInfoType::STRING)
    { // values don't matter
        return true;
    }
    else if (a->type == TypeInfoType::NUMBER)
    {
        auto int_a = static_cast<NumberTypeInfo *>(a);
        auto int_b = static_cast<NumberTypeInfo *>(b);

        if (int_a->size == int_b->size && int_a->number_type == int_b->number_type)
            return true;

        return false;
    }
    else if (a->type == TypeInfoType::FN)
    {
        auto fn_a = static_cast<FnTypeInfo *>(a);
        auto fn_b = static_cast<FnTypeInfo *>(b);

        if (type_match(fn_a->return_type, fn_b->return_type))
        {
            for (i32 i = 0; i < fn_a->args.size(); i++)
            {
                if (type_match(fn_a->args.at(i), fn_b->args.at(i)))
                {}
                else
                { return false; }
            }

            return true;
        }

        return false;
    }
    else if (a->type == TypeInfoType::STRUCT)
    {
        auto struct_a = static_cast<StructTypeInfo *>(a);
        auto struct_b = static_cast<StructTypeInfo *>(b);

        // symbol table does not copy struct type infos
        // so pointer to one struct instance of a type
        // is ALWAYS the same as another
        return struct_a == struct_b;
    }
    else if (a->type == TypeInfoType::POINTER)
    {
        auto ptr_a = static_cast<PointerTypeInfo *>(a);
        auto ptr_b = static_cast<PointerTypeInfo *>(b);

        return type_match(ptr_a->to, ptr_b->to);
    }
    else if (a->type == TypeInfoType::STRUCT_INSTANCE)
    {
        auto ptr_a = static_cast<StructInstanceTypeInfo *>(a);
        auto ptr_b = static_cast<StructInstanceTypeInfo *>(b);
        if (!type_match(ptr_a->struct_type, ptr_b->struct_type))
        { return false; }

        return true;
    }

    panic("Cannot type check this type info");
    return false;
}