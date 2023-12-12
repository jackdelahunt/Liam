#include "type_checker.h"

#include <assert.h>
#include <format>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "ast.h"
#include "baseLayer/debug.h"
#include "compilation_unit.h"
#include "errors.h"
#include "liam.h"
#include "utils.h"

TypeChecker::TypeChecker() {
    this->compilation_unit   = NULL;
    this->compilation_bundle = NULL;
    this->scopes             = std::list<Scope>();
}

void TypeChecker::new_scope() {
    this->scopes.push_back(Scope());
}

void TypeChecker::delete_scope() {
    ASSERT_MSG(this->scopes.size() > 0, "Must be an active scope to add to");
    this->scopes.pop_back();
}

void TypeChecker::add_to_scope(TokenIndex token_index, TypeInfo *type_info) {
    ASSERT_MSG(this->scopes.size() > 0, "Must be an active scope to add to");
    std::string identifier           = this->compilation_unit->get_token_string_from_index(token_index);
    this->scopes.front()[identifier] = type_info;
}

TypeInfo *TypeChecker::get_from_scope(TokenIndex token_index) {
    ASSERT_MSG(this->scopes.size() > 0, "Must be an active scope to add to");
    std::string identifier = this->compilation_unit->get_token_string_from_index(token_index);

    // look into all of the local scopes going up
    // the chain
    for (auto iter = this->scopes.rbegin(); iter != this->scopes.rend(); iter++)
    {
        if (iter->count(identifier) > 0)
        {
            return (*iter)[identifier];
        }
    }

    // if there is nothing then check the fn scope
    TypeInfo *type_info = this->compilation_unit->get_fn_from_scope(token_index);
    if (type_info != NULL)
    {
        return type_info;
    }

    return this->compilation_unit->get_namespace_from_scope(token_index);
}

void TypeChecker::type_check(CompilationBundle *bundle) {
    this->compilation_bundle = bundle;
    for (CompilationUnit *cu : bundle->compilation_units)
    {
        this->compilation_unit = cu;

        // add symbols for structs and fns
        for (auto stmt : this->compilation_unit->top_level_import_statements)
        {
            TRY_CALL_VOID(type_check_import_statement(stmt));
        }
    }

    for (CompilationUnit *cu : bundle->compilation_units)
    {
        this->compilation_unit = cu;

        // add symbols for structs and fns
        for (auto stmt : this->compilation_unit->top_level_struct_statements)
        {
            TRY_CALL_VOID(type_check_struct_symbol(stmt));
        }

        for (auto stmt : this->compilation_unit->top_level_fn_statements)
        {
            TRY_CALL_VOID(type_check_fn_symbol(stmt));
        }
    }

    for (CompilationUnit *cu : bundle->compilation_units)
    {
        this->compilation_unit = cu;

        // type the body of the structs and the declarations of the fns
        for (auto stmt : this->compilation_unit->top_level_struct_statements)
        {
            TRY_CALL_VOID(type_check_struct_statement_full(stmt));
        }

        for (auto stmt : this->compilation_unit->top_level_fn_statements)
        {
            TRY_CALL_VOID(type_check_fn_decl(stmt));
        }
    }

    // type check struct bodies looking for recursive types
    // and then use a topological sort
    std::vector<StructStatement *> all_struct_statements;
    for (CompilationUnit *cu : bundle->compilation_units)
    {
        for (StructStatement *stmt : cu->top_level_struct_statements)
        {
            all_struct_statements.push_back(stmt);
        }
    }

    this->compilation_bundle->sorted_types = TRY_CALL(topilogical_sort(all_struct_statements));

    for (CompilationUnit *cu : bundle->compilation_units)
    {
        this->compilation_unit = cu;

        // finally do the function body pass
        for (auto stmt : this->compilation_unit->top_level_fn_statements)
        {
            TRY_CALL_VOID(type_check_fn_statement_full(stmt));
        }
    }
}

void TypeChecker::type_check_import_statement(ImportStatement *statement) {
    // trimming the " from either side of the token, this is becase it is a string literal
    // and that means
    std::string import_path = this->compilation_unit->get_token_string_from_index(statement->string_literal);
    trim(import_path, "\"");

    std::filesystem::path this_compilation_unit_parent_dir_path =
        this->compilation_unit->file_data->absolute_path.parent_path();
    Option<u64> compilation_unit_index = this->compilation_bundle->get_compilation_unit_index_with_path_relative_from(
        this_compilation_unit_parent_dir_path.string(), import_path
    );

    if (!compilation_unit_index.is_some())
    {
        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message(std::format("Cannot find file with import path '{}' ", import_path))
            .report();

        return;
    }

    NamespaceTypeInfo *type_info = new NamespaceTypeInfo(compilation_unit_index.value());

    ScopeActionStatus status = this->compilation_unit->add_namespace_to_scope(statement->identifier, type_info);

    if (status == ScopeActionStatus::ALREADY_EXISTS)
    {
        std::string identifier = this->compilation_unit->get_token_string_from_index(statement->identifier);
        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message(std::format("Duplicate creation of namespace identifier '{}' ", identifier))
            .report();

        return;
    }

    statement->namespace_type_info = type_info;
}

void TypeChecker::type_check_fn_symbol(FnStatement *statement) {
    ScopeActionStatus status = this->compilation_unit->add_fn_to_scope(statement->identifier, new FnTypeInfo(NULL, {}));
    if (status == ScopeActionStatus::ALREADY_EXISTS)
    {
        std::string identifier = this->compilation_unit->get_token_string_from_index(statement->identifier);
        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message(std::format("Duplicate creation of fn '{}' ", identifier))
            .report();

        return;
    }
}

void TypeChecker::type_check_struct_symbol(StructStatement *statement) {
    // we cannot be sure if we type check all of the struct that all members
    // will be resolved. As structs after this one might be referenced
    // add it to the table and leave its type info blank until we type check it

    ScopeActionStatus status =
        this->compilation_unit->add_type_to_scope(statement->identifier, new StructTypeInfo(statement, {}));
    if (status == ScopeActionStatus::ALREADY_EXISTS)
    {
        std::string identifier = this->compilation_unit->get_token_string_from_index(statement->identifier);
        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message(std::format("Duplicate creation of type '{}' ", identifier))
            .report();

        return;
    }
}

void TypeChecker::type_check_fn_decl(FnStatement *statement) {
    auto param_type_infos = std::vector<TypeInfo *>();
    for (auto &[identifier, expr] : statement->params)
    {
        TRY_CALL_VOID(type_check_type_expression(expr))
        param_type_infos.push_back(expr->type_info);
    }

    TRY_CALL_VOID(type_check_type_expression(statement->return_type));

    std::string name_as_string = this->compilation_unit->get_token_string_from_index(statement->identifier);

    // the current scope is always the fn scope
    auto current_type_info = (FnTypeInfo *)this->compilation_unit->get_fn_from_scope(statement->identifier);
    ASSERT(current_type_info);
    current_type_info->return_type = statement->return_type->type_info;
    current_type_info->args        = param_type_infos;
}

void TypeChecker::type_check_fn_statement_full(FnStatement *statement) {
    // getting the already existing type info for the function is done in 2 ways
    // 1. looking into the symbol table of the current scope (normal function)
    // 2. looking into the member functions of a type (member function)
    FnTypeInfo *fn_type_info = NULL;

    fn_type_info = (FnTypeInfo *)this->compilation_unit->get_fn_from_scope(statement->identifier);
    ASSERT(fn_type_info);

    // params and get type expressions
    auto args = std::vector<std::tuple<TokenIndex, TypeInfo *>>();
    args.reserve(fn_type_info->args.size());
    args.resize(fn_type_info->args.size());
    for (u64 i = 0; i < fn_type_info->args.size(); i++)
    {
        auto &[identifier, expr] = statement->params.at(i);
        args[i]                  = {identifier, fn_type_info->args.at(i)};
    }

    this->new_scope();
    for (auto &[token_index, type_info] : args)
    {
        this->add_to_scope(token_index, type_info);
    }
    TRY_CALL_VOID(type_check_scope_statement(statement->body));
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
                    TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
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
                        compilation_unit->file_data->absolute_path.string(), rt->expression, NULL,
                        statement->return_type, NULL, "Mismatch types in function, return types do not match"
                    );
                    return;
                }
            }
        }
    }
}

void TypeChecker::type_check_struct_statement_full(StructStatement *statement) {
    auto members_type_info = std::vector<std::tuple<std::string, TypeInfo *>>();
    members_type_info.reserve(statement->members.size());
    members_type_info.resize(statement->members.size());
    for (u64 i = 0; i < statement->members.size(); i++)
    {
        auto [member, expr] = statement->members.at(i);
        TRY_CALL_VOID(type_check_type_expression(expr));
        std::string member_string = this->compilation_unit->get_token_string_from_index(member);
        members_type_info[i]      = {member_string, expr->type_info};
    }

    StructTypeInfo *struct_type_info =
        (StructTypeInfo *)this->compilation_unit->get_type_from_scope(statement->identifier);
    ASSERT(struct_type_info != NULL);
    struct_type_info->members = members_type_info;

    statement->type_info = struct_type_info;
}

void TypeChecker::type_check_statement(Statement *statement) {
    switch (statement->statement_type)
    {
    case StatementType::STATEMENT_RETURN:
        return type_check_return_statement(static_cast<ReturnStatement *>(statement));
    case StatementType::STATEMENT_BREAK:
        return type_check_break_statement(static_cast<BreakStatement *>(statement));
    case StatementType::STATEMENT_ASSIGNMENT:
        return type_check_assigment_statement(static_cast<AssigmentStatement *>(statement));
    case StatementType::STATEMENT_EXPRESSION:
        return type_check_expression_statement(static_cast<ExpressionStatement *>(statement));
    case StatementType::STATEMENT_LET:
        return type_check_let_statement(static_cast<LetStatement *>(statement));
    case StatementType::STATEMENT_FOR:
        return type_check_for_statement(static_cast<ForStatement *>(statement));
    case StatementType::STATEMENT_IF:
        return type_check_if_statement(static_cast<IfStatement *>(statement));
    case StatementType::STATEMENT_CONTINUE:
        break;
    case StatementType::STATEMENT_STRUCT:
    case StatementType::STATEMENT_FN:
        ASSERT_MSG(0, "These should of already been type checked in the first pass");
    default:
        ASSERT_MSG(
            0, "Statement not implemented in type checker, id -> " + std::to_string((int)statement->statement_type)
        );
    }
}

void TypeChecker::type_check_return_statement(ReturnStatement *statement) {
    if (statement->expression)
        TRY_CALL_VOID(type_check_expression(statement->expression));
}

void TypeChecker::type_check_break_statement(BreakStatement *statement) {
}

void TypeChecker::type_check_let_statement(LetStatement *statement) {
    TRY_CALL_VOID(type_check_expression(statement->rhs));

    // if let type is there type match both and set var type
    // to the let type... else just set it to the rhs
    if (statement->type)
    {
        TRY_CALL_VOID(type_check_type_expression(statement->type));
        if (!type_match(statement->type->type_info, statement->rhs->type_info))
        {
            ErrorReporter::report_type_checker_error(
                compilation_unit->file_data->absolute_path.string(), statement->rhs, NULL, statement->type, NULL,
                "Mismatched types in let statement"
            );
            return;
        }
    }

    //    auto string = this->compilation_unit->get_token_string_from_index(statement->identifier);
    //    TRY_CALL_VOID(symbol_table->add_identifier_type(string, statement->rhs->type_info));

    this->add_to_scope(statement->identifier, statement->rhs->type_info);
}

void TypeChecker::type_check_scope_statement(ScopeStatement *statement) {
    for (auto stmt : statement->statements)
    {
        TRY_CALL_VOID(type_check_statement(stmt));
    }
}

void TypeChecker::type_check_for_statement(ForStatement *statement) {
    TRY_CALL_VOID(type_check_statement(statement->assign));
    TRY_CALL_VOID(type_check_expression(statement->condition));
    TRY_CALL_VOID(type_check_statement(statement->update));

    if (statement->condition->type_info->type != TypeInfoType::BOOLEAN)
    {
        panic("Second statement in for loop needs to evaluate to a bool");
    }

    TRY_CALL_VOID(type_check_scope_statement(statement->body));
}

void TypeChecker::type_check_if_statement(IfStatement *statement) {
    TRY_CALL_VOID(type_check_expression(statement->expression));

    if (!type_match(statement->expression->type_info, this->compilation_unit->global_type_scope["bool"]))
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->absolute_path.string(), statement->expression, NULL, NULL, NULL,
            "can only pass boolean expressions to if statements"
        );
        return;
    }

    this->new_scope();
    TRY_CALL_VOID(type_check_scope_statement(statement->body));
    this->delete_scope();

    if (statement->else_statement)
    {
        // might not have else statement
        // not using symbol table copy either as symbols
        // should not leak to sub statements
        TRY_CALL_VOID(type_check_else_statement(statement->else_statement));
    }
}

void TypeChecker::type_check_else_statement(ElseStatement *statement) {
    if (statement->if_statement)
    {
        TRY_CALL_VOID(type_check_if_statement(statement->if_statement));
    }

    if (statement->body)
    {
        TRY_CALL_VOID(type_check_scope_statement(statement->body));
    }
}

void TypeChecker::type_check_assigment_statement(AssigmentStatement *statement) {
    TRY_CALL_VOID(type_check_expression(statement->lhs));
    TRY_CALL_VOID(type_check_expression(statement->assigned_to->expression));

    if (!type_match(statement->lhs->type_info, statement->assigned_to->expression->type_info))
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->absolute_path.string(), statement->lhs, statement->assigned_to->expression,
            NULL, NULL, "Type mismatch, trying to assign a identifier to an expression of different type"
        );
        return;
    }
}

void TypeChecker::type_check_expression_statement(ExpressionStatement *statement) {
    TRY_CALL_VOID(type_check_expression(statement->expression));
}

void TypeChecker::type_check_expression(Expression *expression) {
    switch (expression->type)
    {
    case ExpressionType::EXPRESSION_STRING_LITERAL:
        return type_check_string_literal_expression(static_cast<StringLiteralExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_NUMBER_LITERAL:
        return type_check_number_literal_expression(static_cast<NumberLiteralExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_BOOL_LITERAL:
        return type_check_bool_literal_expression(static_cast<BoolLiteralExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_CALL:
        return type_check_call_expression(static_cast<CallExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_IDENTIFIER:
        return type_check_identifier_expression(static_cast<IdentifierExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_BINARY:
        return type_check_binary_expression(static_cast<BinaryExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_UNARY:
        return type_check_unary_expression(static_cast<UnaryExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_GET:
        return type_check_get_expression(static_cast<GetExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_GROUP:
        return type_check_group_expression(static_cast<GroupExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_NULL_LITERAL:
        return type_check_null_literal_expression(static_cast<NullLiteralExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_ZERO_LITERAL:
        return type_check_zero_literal_expression(static_cast<ZeroLiteralExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_INSTANTIATION:
        return type_check_instantiate_expression(static_cast<InstantiateExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_STRUCT_INSTANCE:
        return type_check_struct_instance_expression(static_cast<StructInstanceExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_STATIC_ARRAY:
        return type_check_static_array_literal_expression(static_cast<StaticArrayExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_SUBSCRIPT:
        return type_check_subscript_expression(static_cast<SubscriptExpression *>(expression));
        break;
    default:
        UNREACHABLE();
    }
}

void TypeChecker::type_check_binary_expression(BinaryExpression *expression) {
    TRY_CALL_VOID(type_check_expression(expression->left));
    TRY_CALL_VOID(type_check_expression(expression->right));

    if (!type_match(expression->left->type_info, expression->right->type_info))
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->absolute_path.string(), expression->left, expression->right, NULL, NULL,
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
                compilation_unit->file_data->absolute_path.string(), expression->left, expression->right, NULL, NULL,
                "Cannot use logical operators on non bool type"
            );
            return;
        }

        info = this->compilation_unit->global_type_scope["bool"];
    }

    // math ops - numbers -> numbers
    if (expression->op == TokenType::TOKEN_PLUS || expression->op == TokenType::TOKEN_STAR ||
        expression->op == TokenType::TOKEN_SLASH || expression->op == TokenType::TOKEN_MOD ||
        expression->op == TokenType::TOKEN_MINUS)
    {
        if (expression->left->type_info->type != TypeInfoType::NUMBER)
        {
            ErrorReporter::report_type_checker_error(
                compilation_unit->file_data->absolute_path.string(), expression->left, expression->right, NULL, NULL,
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
                compilation_unit->file_data->absolute_path.string(), expression->left, expression->right, NULL, NULL,
                "Cannot use comparison operator on non number"
            );
            return;
        }
        info = this->compilation_unit->global_type_scope["bool"];
    }

    // compare - any -> bool
    if (expression->op == TokenType::TOKEN_EQUAL || expression->op == TokenType::TOKEN_NOT_EQUAL)
    {
        info = this->compilation_unit->global_type_scope["bool"];
    }

    assert(info != NULL);

    expression->type_info = info;
}

void TypeChecker::type_check_string_literal_expression(StringLiteralExpression *expression) {
    expression->type_info = this->compilation_unit->global_type_scope["str"];
}

void TypeChecker::type_check_number_literal_expression(NumberLiteralExpression *expression) {

    std::string token_string = this->compilation_unit->get_token_string_from_index(expression->token);

    auto [number, type, size] = extract_number_literal_size(token_string);
    if (size == -1)
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->absolute_path.string(), expression, NULL, NULL, NULL,
            "Problem parsing number literal"
        );
        return;
    }

    // TODO I have no idea what this does
    if (type != NumberType::FLOAT && number != (i64)number)
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->absolute_path.string(), expression, NULL, NULL, NULL,
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
                compilation_unit->file_data->absolute_path.string(), expression, NULL, NULL, NULL,
                "Unsigned number cannot be negative"
            );
            return;
        }

        if (size == 8)
        {
            expression->type_info = this->compilation_unit->global_type_scope["u8"];
        }
        else if (size == 16)
        {
            expression->type_info = this->compilation_unit->global_type_scope["u16"];
        }
        else if (size == 32)
        {
            expression->type_info = this->compilation_unit->global_type_scope["u32"];
        }
        else if (size == 64)
        {
            expression->type_info = this->compilation_unit->global_type_scope["u64"];
        }
    }
    break;
    case NumberType::SIGNED: {

        if (size == 8)
        {
            expression->type_info = this->compilation_unit->global_type_scope["i8"];
        }
        else if (size == 16)
        {
            expression->type_info = this->compilation_unit->global_type_scope["i16"];
        }
        else if (size == 32)
        {
            expression->type_info = this->compilation_unit->global_type_scope["i32"];
        }
        else if (size == 64)
        {
            expression->type_info = this->compilation_unit->global_type_scope["i64"];
        }
    }
    break;
    case NumberType::FLOAT: {
        if (size == 8 || size == 16)
        {
            ErrorReporter::report_type_checker_error(
                compilation_unit->file_data->absolute_path.string(), expression, NULL, NULL, NULL,
                "Cannot create float of this size can only use 32 and 64 sizes"
            );
            return;
        }
        else if (size == 32)
        {
            expression->type_info = this->compilation_unit->global_type_scope["f32"];
        }
        else if (size == 64)
        {
            expression->type_info = this->compilation_unit->global_type_scope["f64"];
        }
    }
    break;
    }
}

void TypeChecker::type_check_bool_literal_expression(BoolLiteralExpression *expression) {
    expression->type_info = new BoolTypeInfo();
}

void TypeChecker::type_check_unary_expression(UnaryExpression *expression) {
    TRY_CALL_VOID(type_check_expression(expression->expression));

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
                compilation_unit->file_data->absolute_path.string(), expression, NULL, NULL, NULL,
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
                compilation_unit->file_data->absolute_path.string(), expression, NULL, NULL, NULL,
                "Cannot use unary operator ! on non-boolean type"
            );
            return;
        }

        expression->type_info = expression->expression->type_info;
        return;
    }

    panic("Internal :: Unrecognised unary operator");
}

void TypeChecker::type_check_call_expression(CallExpression *expression) {

    TRY_CALL_VOID(type_check_expression(expression->callee));
    auto callee_expression = expression->callee;

    if (expression->callee->type_info->type == TypeInfoType::FN)
    {
        TRY_CALL_VOID(type_check_fn_call_expression(expression));
        return;
    }

    ErrorReporter::report_type_checker_error(
        compilation_unit->file_data->absolute_path.string(), callee_expression, NULL, NULL, NULL,
        "Can only call functions"
    );
}

void TypeChecker::type_check_fn_call_expression(CallExpression *expression) {

    auto callee_expression = expression->callee;

    auto arg_type_infos = std::vector<TypeInfo *>();
    for (auto arg : expression->args)
    {
        TRY_CALL_VOID(type_check_expression(arg));
        arg_type_infos.push_back(arg->type_info);
    }

    auto fn_type_info = static_cast<FnTypeInfo *>(callee_expression->type_info);
    if (fn_type_info->args.size() != arg_type_infos.size())
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->absolute_path.string(), callee_expression, NULL, NULL, NULL,
            std::format(
                "Incorrect number of arguments in call expression, expected {} got {}", fn_type_info->args.size(),
                arg_type_infos.size()
            )
        );
        return;
    }

    for (u64 i = 0; i < fn_type_info->args.size(); i++)
    {
        if (!type_match(fn_type_info->args.at(i), arg_type_infos.at(i)))
        {
            ErrorReporter::report_type_checker_error(
                compilation_unit->file_data->absolute_path.string(), callee_expression, expression->args.at(i), NULL,
                NULL, "Mismatched types function call"
            );
            return;
        }
    }

    expression->type_info = fn_type_info->return_type;
}

void TypeChecker::type_check_identifier_expression(IdentifierExpression *expression) {
    TypeInfo *type_info = this->get_from_scope(expression->identifier);
    if (type_info == NULL)
    {
        std::string identifier = this->compilation_unit->get_token_string_from_index(expression->identifier);
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->absolute_path.string(), expression, NULL, NULL, NULL,
            std::format("Unrecognized identifier \"{}\"", identifier)
        );
        return;
    }

    expression->type_info = type_info;
}

void TypeChecker::type_check_get_expression(GetExpression *expression) {
    TRY_CALL_VOID(type_check_expression(expression->lhs));

    if (expression->lhs->type_info->type == TypeInfoType::NAMESPACE)
    {
        NamespaceTypeInfo *namespace_type_info = (NamespaceTypeInfo *)expression->lhs->type_info;
        CompilationUnit *namespace_compilation_unit =
            this->compilation_bundle->compilation_units[namespace_type_info->compilation_unit_index];

        std::string identifier     = this->compilation_unit->get_token_string_from_index(expression->member);
        TypeInfo *member_type_info = namespace_compilation_unit->get_fn_from_scope_with_string(identifier);

        if (member_type_info == NULL)
        {
            ErrorReporter::report_type_checker_error(
                this->compilation_unit->file_data->absolute_path.string(), expression->lhs, NULL, NULL, NULL,
                std::format("no symbol '{}' found in namespace", identifier)
            );
            return;
        }

        expression->type_info = member_type_info;
        return;
    }

    TypeInfo *using_type = expression->lhs->type_info;

    // if it is a pointer then we dereference it one layer deep
    // ^T.get() --> T.get()
    // ^^T.get() --> ^T.get() :: won't go all the layers down
    if (expression->lhs->type_info->type == TypeInfoType::POINTER)
    {
        using_type = ((PointerTypeInfo *)expression->lhs->type_info)->to;
    }

    if (using_type->type == TypeInfoType::STRUCT)
    {
        StructTypeInfo *struct_type_info = (StructTypeInfo *)using_type;

        std::string member_string  = this->compilation_unit->get_token_string_from_index(expression->member);
        TypeInfo *member_type_info = NULL;
        for (auto [identifier, member_type] : struct_type_info->members)
        {
            if (identifier == member_string)
            {
                member_type_info = member_type;
                break;
            }
        }

        if (member_type_info == NULL)
        {
            ErrorReporter::report_type_checker_error(
                compilation_unit->file_data->absolute_path.string(), expression, NULL, NULL, NULL,
                std::format("Cannot find member \"{}\" in struct", member_string)
            );
            return;
        }

        expression->type_info = member_type_info;
        return;
    }

    if (using_type->type == TypeInfoType::STATIC_ARRAY)
    {
        std::string member_string = this->compilation_unit->get_token_string_from_index(expression->member);

        if (compare_string(member_string, "size"))
        {
            expression->type_info = this->compilation_unit->global_type_scope["u64"];
            return;
        }

        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message(
                std::format("can only use 'size' builtin member for static arrays '{}' does not exist", member_string)
            )
            .set_expr_1(expression)
            .report();

        return;
    }

    ErrorReporter::report_type_checker_error(
        this->compilation_unit->file_data->absolute_path, expression->lhs, NULL, NULL, NULL,
        "Cannot derive member from non struct/namespace/array type"
    );
    return;
}

void TypeChecker::type_check_group_expression(GroupExpression *expression) {
    TRY_CALL_VOID(type_check_expression(expression->expression));
    expression->type_info = expression->expression->type_info;
}

void TypeChecker::type_check_null_literal_expression(NullLiteralExpression *expression) {
    expression->type_info = new PointerTypeInfo(new AnyTypeInfo());
}

void TypeChecker::type_check_zero_literal_expression(ZeroLiteralExpression *expression) {
    expression->type_info = new AnyTypeInfo{TypeInfoType::ANY};
}

void TypeChecker::type_check_instantiate_expression(InstantiateExpression *expression) {

    // TODO
    // remove the idea of an instantiate expression, we are parsing it
    // as if we are always assuming it is a struct instance anyway
    // just think of every as a "new Struct{...}" an no other type
    // this is a leftover from "new Enum{...}"
    if (expression->expression->type != ExpressionType::EXPRESSION_STRUCT_INSTANCE)
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->absolute_path.string(), expression, expression->expression, NULL, NULL,
            "Cannot instantiate non-struct type"
        );
        return;
    }

    TRY_CALL_VOID(type_check_expression(expression->expression));

    expression->type_info = expression->expression->type_info;
}

void TypeChecker::type_check_struct_instance_expression(StructInstanceExpression *expression) {
    TRY_CALL_VOID(type_check_type_expression(expression->type_expression));

    TypeInfo *type_info = expression->type_expression->type_info;
    // check it's a struct
    if (type_info->type != TypeInfoType::STRUCT)
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->absolute_path.string(), expression, NULL, NULL, NULL,
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
        TRY_CALL_VOID(type_check_expression(expr));
        calling_args_type_infos.emplace_back(name, expr->type_info);
    }

    // check counts
    if (struct_type_info->members.size() != calling_args_type_infos.size())
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->absolute_path.string(), expression, NULL, NULL, NULL,
            std::format(
                "Incorrect number of arguments in new expression, expected {} got {}", struct_type_info->members.size(),
                calling_args_type_infos.size()
            )
        );
        return;
    }

    // check types
    for (u64 i = 0; i < calling_args_type_infos.size(); i++)
    {
        // new expression member and type info
        auto [expression_member, expression_type] = calling_args_type_infos.at(i);

        // struct member and type info
        auto [member, type] = struct_type_info->members.at(i);

        if (expression_member != member)
        {
            auto [name, expr] = expression->named_expressions.at(i);
            ErrorReporter::report_type_checker_error(
                compilation_unit->file_data->absolute_path.string(), expr, NULL, NULL, NULL,
                "Incorrect name specifier in new expression"
            );
            return;
        }
    }

    expression->type_info = struct_type_info;
}

void TypeChecker::type_check_static_array_literal_expression(StaticArrayExpression *expression) {
    TRY_CALL_VOID(type_check_expression(expression->number));
    TRY_CALL_VOID(type_check_type_expression(expression->type_expression));

    if (expression->expressions.size() != expression->number->number)
    {
        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message(std::format(
                "static array literal expects {} values but got {}", expression->number->number,
                expression->expressions.size()
            ))
            .set_expr_1(expression->number)
            .set_type_expr_1(expression->type_expression)
            .report();

        return;
    }

    for (Expression *expr : expression->expressions)
    {
        TRY_CALL_VOID(type_check_expression(expr));
        if (!type_match(expression->type_expression->type_info, expr->type_info))
        {
            TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
                .set_message("mistmaatched types in static array literal")
                .set_expr_1(expr)
                .set_type_expr_1(expression->type_expression)
                .report();

            return;
        }
    }

    expression->type_info = new StaticArrayTypeInfo(expression->number->number, expression->type_expression->type_info);
}

void TypeChecker::type_check_subscript_expression(SubscriptExpression *expression) {
    TRY_CALL_VOID(type_check_expression(expression->subscriptee));
    TRY_CALL_VOID(type_check_expression(expression->subscripter));

    if (expression->subscriptee->type_info->type != TypeInfoType::STATIC_ARRAY)
    {
        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message("can only subscript array types")
            .set_expr_1(expression->subscriptee)
            .set_expr_2(expression->subscripter)
            .report();

        return;
    }

    if (expression->subscripter->type_info->type != TypeInfoType::NUMBER)
    {
        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message("can only subscript with number types")
            .set_expr_2(expression->subscripter)
            .report();

        return;
    }

    NumberTypeInfo *number_type_info = (NumberTypeInfo *)expression->subscripter->type_info;
    if (number_type_info->number_type == NumberType::FLOAT)
    {
        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message("cannot use a float type to subscript an array")
            .set_expr_2(expression->subscripter)
            .report();

        return;
    }

    StaticArrayTypeInfo *array_type_info = (StaticArrayTypeInfo *)expression->subscriptee->type_info;
    expression->type_info                = array_type_info->base_type;
}

void TypeChecker::type_check_type_expression(TypeExpression *type_expression) {
    switch (type_expression->type)
    {
    case TypeExpressionType::TYPE_IDENTIFIER:
        type_check_identifier_type_expression(static_cast<IdentifierTypeExpression *>(type_expression));
        break;
    case TypeExpressionType::TYPE_UNARY:
        type_check_unary_type_expression(static_cast<UnaryTypeExpression *>(type_expression));
        break;
    case TypeExpressionType::TYPE_GET:
        type_check_get_type_expression(static_cast<GetTypeExpression *>(type_expression));
        break;
    case TypeExpressionType::TYPE_STATIC_ARRAY:
        type_check_static_array_type_expression(static_cast<StaticArrayTypeExpression *>(type_expression));
        break;
    default:
        UNREACHABLE();
    }
}

void TypeChecker::type_check_unary_type_expression(UnaryTypeExpression *type_expression) {
    if (type_expression->unary_type == UnaryType::POINTER)
    {
        TRY_CALL_VOID(type_check_type_expression(type_expression->type_expression));
        type_expression->type_info = new PointerTypeInfo(type_expression->type_expression->type_info);
        return;
    }

    panic("Internal :: Cannot type check this operator yet...");
}

void TypeChecker::type_check_identifier_type_expression(IdentifierTypeExpression *type_expression) {
    // first checking the type table then checking any namespaces
    TypeInfo *type_info = this->compilation_unit->get_type_from_scope(type_expression->identifier);
    if (type_info == NULL)
    {
        type_info = this->compilation_unit->get_namespace_from_scope(type_expression->identifier);
    }

    if (type_info == NULL)
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->absolute_path.string(), NULL, NULL, type_expression, NULL,
            "Unrecognised identifier in type expression, no type or namespace exists with this name"
        );
        return;
    }

    type_expression->type_info = type_info;
}

void TypeChecker::type_check_get_type_expression(GetTypeExpression *type_expression) {
    TRY_CALL_VOID(type_check_type_expression(type_expression->type_expression));
    if (type_expression->type_expression->type_info->type != TypeInfoType::NAMESPACE)
    {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->absolute_path.string(), NULL, NULL, type_expression->type_expression, NULL,
            "Can only use '.' on namespace identifiers in type expressions"
        );
        return;
    }

    NamespaceTypeInfo *namespace_type_info = (NamespaceTypeInfo *)type_expression->type_expression->type_info;
    CompilationUnit *other_compilation_unit =
        this->compilation_bundle->compilation_units[namespace_type_info->compilation_unit_index];

    // getting the string of the identifier from the current compilation unit
    // but then looking that up in the other compilation unit
    std::string identifier = this->compilation_unit->get_token_string_from_index(type_expression->identifier);
    TypeInfo *type_info    = other_compilation_unit->get_type_from_scope_with_string(identifier);

    if (type_info == NULL)
    {
        ErrorReporter::report_type_checker_error(
            this->compilation_unit->file_data->absolute_path.string(), NULL, NULL, type_expression->type_expression,
            NULL, std::format("no symbol '{}' found in namespace", identifier)
        );
        return;
    }

    type_expression->type_info = type_info;
}

void TypeChecker::type_check_static_array_type_expression(StaticArrayTypeExpression *type_expression) {
    TRY_CALL_VOID(type_check_type_expression(type_expression->base_type));
    TRY_CALL_VOID(type_check_number_literal_expression(type_expression->size));

    type_expression->type_info =
        new StaticArrayTypeInfo(type_expression->size->number, type_expression->base_type->type_info);
}

bool type_match(TypeInfo *a, TypeInfo *b) {

    ASSERT_MSG(!(a->type == TypeInfoType::ANY && b->type == TypeInfoType::ANY), "Cannot compare 2 any types");

    if (a->type != b->type)
    {
        if (a->type != TypeInfoType::ANY && b->type != TypeInfoType::ANY)
        {
            return false;
        }
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
            for (u64 i = 0; i < fn_a->args.size(); i++)
            {
                if (!type_match(fn_a->args.at(i), fn_b->args.at(i)))
                {
                    return false;
                }
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
    else if (a->type == TypeInfoType::STATIC_ARRAY)
    {
        auto array_a = static_cast<StaticArrayTypeInfo *>(a);
        auto array_b = static_cast<StaticArrayTypeInfo *>(b);

        if (array_a->size != array_b->size)
        {
            return false;
        }

        return type_match(array_a->base_type, array_b->base_type);
    }

    panic("Cannot type check this type info");
    return false;
}

std::tuple<i64, NumberType, i32> extract_number_literal_size(std::string literal) {

#define BAD_PARSE                                                                                                      \
    { 0, NumberType::UNSIGNED, -1 }

    int literal_end = 0;
    while (literal_end < literal.size() &&
           (is_digit(literal.at(literal_end)) || literal.at(literal_end) == '-' || literal.at(literal_end) == '.'))
    {
        literal_end++;
    }

    // literal == 0..literal_end
    // type == literal_end..literal_end +1
    // postfix == literal_end + 1..string_end

    auto literal_string = literal.substr(0, literal_end);

    std::string type_string    = "";
    std::string postfix_string = "";

    // if there is a postfix notation
    // else just infer a i64
    if (literal_end != literal.size())
    {
        type_string    = literal.substr(literal_end, 1);
        postfix_string = literal.substr(literal_end + 1, literal.size() - literal_end);
    }
    else
    {
        auto n = std::stod(literal_string);
        return {n, NumberType::SIGNED, 64};
    }

    int size;

    try
    { size = std::stoi(postfix_string); }
    catch (std::exception &e)
    { return BAD_PARSE; }

    NumberType type;

    if (type_string == "u")
    {
        type = NumberType::UNSIGNED;
    }
    else if (type_string == "f")
    {
        type = NumberType::FLOAT;
    }
    else if (type_string == "i")
    {
        type = NumberType::SIGNED;
    }
    else
    {
        return BAD_PARSE;
    }

    if (size == 8 || size == 16 || size == 32 || size == 64)
    {
        try
        {
            auto n = std::stod(literal_string);
            return {n, type, size};
        }
        catch (std::exception &e)
        {}
    }

    return BAD_PARSE;
}

void add_type_info_to_map(
    std::vector<SortingNode> *nodes, std::unordered_map<StructTypeInfo *, u64> *type_info_to_node_index_map,
    StructTypeInfo *type_info
) {
    SortingNode node = SortingNode(type_info);
    u64 node_index   = nodes->size();
    nodes->push_back(node);
    (*type_info_to_node_index_map)[type_info] = node_index;
}

void add_dependent_types_to_list(TypeInfo *type_info, std::vector<StructTypeInfo *> *dependent_types) {
    if (type_info->type == TypeInfoType::STRUCT)
    {
        dependent_types->push_back((StructTypeInfo *)type_info);
        for (auto [_, child] : ((StructTypeInfo *)type_info)->members)
        {
            add_dependent_types_to_list(child, dependent_types);
        }
    }
    else if (type_info->type == TypeInfoType::STATIC_ARRAY)
    {
        TypeInfo *child_type_info = ((StaticArrayTypeInfo *)type_info)->base_type;
        add_dependent_types_to_list(child_type_info, dependent_types);
    }
}

void resolve_dependencies(
    std::vector<SortingNode> *nodes, std::unordered_map<StructTypeInfo *, u64> *type_info_to_node_index_map,
    SortingNode &current_node
) {
    std::vector<StructTypeInfo *> dependent_types;
    for (auto &child : current_node.type_info->members)
    {
        auto [name, child_type_info] = child;
        add_dependent_types_to_list(child_type_info, &dependent_types);
    }

    for (StructTypeInfo *child : dependent_types)
    {
        ASSERT(type_info_to_node_index_map->count(child) > 0);

        u64 child_index         = (*type_info_to_node_index_map)[child];
        SortingNode *child_node = &nodes->at(child_index);
        current_node.depends_on.push_back(child_node);
    }
}

void topological_visit(std::vector<SortingNode> *L, SortingNode *node) {
    if (node->permenent_mark)
        return;

    if (node->temperory_mark)
    {
        // TODO this error is terrible
        CompilationUnit *compilation_unit = node->type_info->defined_location->compilation_unit;
        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message(std::format(
                "recursive type found in {}",
                compilation_unit->get_token_string_from_index(node->type_info->defined_location->identifier)
            ))
            .report();

        return;
    }

    node->temperory_mark = true;

    for (SortingNode *child : node->depends_on)
    {
        topological_visit(L, child);
    }

    node->temperory_mark = false;
    node->permenent_mark = true;
    L->push_back(*node); // TODO maybe we dont need to do a copy here????
}

std::vector<SortingNode> topilogical_sort(std::vector<StructStatement *> statements) {
    std::vector<SortingNode> nodes;
    std::unordered_map<StructTypeInfo *, u64> type_info_to_node_index_map;

    // convert all struct statements into the nodes we need for the graph
    for (auto struct_statement : statements)
    {
        add_type_info_to_map(&nodes, &type_info_to_node_index_map, struct_statement->type_info);
    }

    // fill in the dependency vector for each sorting node
    for (SortingNode &sorting_node : nodes)
    {
        resolve_dependencies(&nodes, &type_info_to_node_index_map, sorting_node);
    }

    // Depth-first search
    // https://en.wikipedia.org/wiki/Topological_sorting
    std::vector<SortingNode> L;
    for (SortingNode &sorting_node : nodes)
    {
        topological_visit(&L, &sorting_node);
    }

    return L;
}
