#include "borrow_checker.h"
#include "args.h"
#include "errors.h"

OwnershipStatus::OwnershipStatus(Owner owner, TypeInfo *type_info, Span ownership_of_value) {
    this->owner              = owner;
    this->type_info          = type_info;
    this->ownership_of_value = ownership_of_value;
}

OwnershipTable::OwnershipTable() {
    this->table = std::map<std::string, OwnershipStatus>();
}

OwnershipTable OwnershipTable::copy() {
    return *this;
}

void OwnershipTable::addOwnedValue(std::string identifier, OwnershipStatus status) {
    this->table.insert({identifier, status});
}

bool OwnershipTable::ownedValueExistedInScope(std::string identifier) {
    return this->table.find(identifier) != this->table.end();
}

OwnershipStatus OwnershipTable::getOwnershipStatus(std::string identifier) {
    return this->table.at(identifier);
}

void OwnershipTable::setOwnershipStatus(std::string identifier, OwnershipStatus status) {
    this->table[identifier] = status;
}

void BorrowChecker::borrow_check(File *file) {

    OwnershipTable empty = OwnershipTable();

    for (auto stmt : file->statements)
    { TRY_CALL(borrow_check_statement(stmt, &empty)); }
}

void BorrowChecker::borrow_check_statement(Statement *statement, OwnershipTable *ownership_table) {
    switch (statement->statement_type)
    {
    case StatementType::STATEMENT_RETURN:
        return borrow_check_return_statement(dynamic_cast<ReturnStatement *>(statement), ownership_table);
        break;
    case StatementType::STATEMENT_BREAK:
        return borrow_check_break_statement(dynamic_cast<BreakStatement *>(statement), ownership_table);
        break;
    case StatementType::STATEMENT_LET:
        return borrow_check_let_statement(dynamic_cast<LetStatement *>(statement), ownership_table);
        break;
    case StatementType::STATEMENT_FN:
        return borrow_check_fn_statement(dynamic_cast<FnStatement *>(statement), ownership_table);
        break;
    case StatementType::STATEMENT_SCOPE:
        return borrow_check_scope_statement(dynamic_cast<ScopeStatement *>(statement), ownership_table);
        break;
    case StatementType::STATEMENT_STRUCT:
        return borrow_check_struct_statement(dynamic_cast<StructStatement *>(statement), ownership_table);
        break;
    case StatementType::STATEMENT_ASSIGNMENT:
        return borrow_check_assigment_statement(dynamic_cast<AssigmentStatement *>(statement), ownership_table);
        break;
    case StatementType::STATEMENT_EXPRESSION:
        return borrow_check_expression_statement(dynamic_cast<ExpressionStatement *>(statement), ownership_table);
        break;
    case StatementType::STATEMENT_FOR:
        return borrow_check_for_statement(dynamic_cast<ForStatement *>(statement), ownership_table);
        break;
    case StatementType::STATEMENT_IF:
        return borrow_check_if_statement(dynamic_cast<IfStatement *>(statement), ownership_table);
        break;
    case StatementType::STATEMENT_ENUM:
        return borrow_check_enum_statement(dynamic_cast<EnumStatement *>(statement), ownership_table);
        break;
    case StatementType::STATEMENT_CONTINUE:
        return borrow_check_continue_statement(dynamic_cast<ContinueStatement *>(statement), ownership_table);
        break;
    case StatementType::STATEMENT_ALIAS:
        return borrow_check_alias_statement(dynamic_cast<AliasStatement *>(statement), ownership_table);
        break;
    case StatementType::STATEMENT_TEST:
        if (args->test)
            return borrow_check_test_statement(dynamic_cast<TestStatement *>(statement), ownership_table);
        break;
    }

    panic("Statement not implemented in borrow checker :[");
}

void BorrowChecker::borrow_check_return_statement(ReturnStatement *statement, OwnershipTable *ownership_table) {
}

void BorrowChecker::borrow_check_break_statement(BreakStatement *statement, OwnershipTable *ownership_table) {
}

void BorrowChecker::borrow_check_let_statement(LetStatement *statement, OwnershipTable *ownership_table) {
    if (statement->rhs->type_info->type == TypeInfoType::OWNED_POINTER)
    {
        ownership_table->addOwnedValue(
            statement->identifier.string,
            OwnershipStatus(OwnershipStatus::SCOPE, statement->rhs->type_info, statement->rhs->span)
        );
    }
}

void BorrowChecker::borrow_check_scope_statement(ScopeStatement *statement, OwnershipTable *ownership_table) {
    for (auto stmt : statement->statements)
    { borrow_check_statement(stmt, ownership_table); }
}

void BorrowChecker::borrow_check_fn_statement(FnStatement *statement, OwnershipTable *ownership_table) {

    if (statement->is_extern)
        return;

    auto ownership_table_copy = ownership_table->copy();

    for (auto [identifier, type] : statement->params)
    {
        if (type->type_info->type == TypeInfoType::OWNED_POINTER)
        {
            ownership_table_copy.addOwnedValue(
                identifier.string, OwnershipStatus(OwnershipStatus::SCOPE, type->type_info, identifier.span)
            );
        }
    }

    borrow_check_scope_statement(statement->body, &ownership_table_copy);
}

void BorrowChecker::borrow_check_struct_statement(StructStatement *statement, OwnershipTable *ownership_table) {
}

void BorrowChecker::borrow_check_assigment_statement(AssigmentStatement *statement, OwnershipTable *ownership_table) {
}

void BorrowChecker::borrow_check_expression_statement(ExpressionStatement *statement, OwnershipTable *ownership_table) {
    borrow_check_expression(statement->expression, ownership_table);
}

void BorrowChecker::borrow_check_for_statement(ForStatement *statement, OwnershipTable *ownership_table) {
}

void BorrowChecker::borrow_check_if_statement(IfStatement *statement, OwnershipTable *ownership_table) {
}

void BorrowChecker::borrow_check_else_statement(ElseStatement *statement, OwnershipTable *ownership_table) {
}

void BorrowChecker::borrow_check_enum_statement(EnumStatement *statement, OwnershipTable *ownership_table) {
}

void BorrowChecker::borrow_check_continue_statement(ContinueStatement *statement, OwnershipTable *ownership_table) {
}

void BorrowChecker::borrow_check_alias_statement(AliasStatement *statement, OwnershipTable *ownership_table) {
}

void BorrowChecker::borrow_check_test_statement(TestStatement *statement, OwnershipTable *ownership_table) {
}

void BorrowChecker::borrow_check_expression(Expression *expression, OwnershipTable *ownership_table) {
    switch (expression->type)
    {
    case ExpressionType::EXPRESSION_STRING_LITERAL:
        borrow_check_string_literal_expression(dynamic_cast<StringLiteralExpression *>(expression), ownership_table);
        break;
    case ExpressionType::EXPRESSION_NUMBER_LITERAL:
        borrow_check_int_literal_expression(dynamic_cast<NumberLiteralExpression *>(expression), ownership_table);
        break;
    case ExpressionType::EXPRESSION_BOOL_LITERAL:
        borrow_check_bool_literal_expression(dynamic_cast<BoolLiteralExpression *>(expression), ownership_table);
        break;
    case ExpressionType::EXPRESSION_CALL:
        borrow_check_call_expression(dynamic_cast<CallExpression *>(expression), ownership_table);
        break;
    case ExpressionType::EXPRESSION_IDENTIFIER:
        borrow_check_identifier_expression(dynamic_cast<IdentifierExpression *>(expression), ownership_table);
        break;
    case ExpressionType::EXPRESSION_BINARY:
        borrow_check_binary_expression(dynamic_cast<BinaryExpression *>(expression), ownership_table);
        break;
    case ExpressionType::EXPRESSION_IS:
        borrow_check_is_expression(dynamic_cast<IsExpression *>(expression), ownership_table);
        break;
    case ExpressionType::EXPRESSION_UNARY:
        borrow_check_unary_expression(dynamic_cast<UnaryExpression *>(expression), ownership_table);
        break;
    case ExpressionType::EXPRESSION_GET:
        borrow_check_get_expression(dynamic_cast<GetExpression *>(expression), ownership_table);
        break;
    case ExpressionType::EXPRESSION_NEW:
        borrow_check_instantiate_expression(dynamic_cast<InstantiateExpression *>(expression), ownership_table);
        break;
    case ExpressionType::EXPRESSION_GROUP:
        borrow_check_group_expression(dynamic_cast<GroupExpression *>(expression), ownership_table);
        break;
    case ExpressionType::EXPRESSION_NULL_LITERAL:
        borrow_check_null_literal_expression(dynamic_cast<NullLiteralExpression *>(expression), ownership_table);
        break;
    case ExpressionType::EXPRESSION_PROPAGATE:
        borrow_check_propagate_expression(dynamic_cast<PropagateExpression *>(expression), ownership_table);
        break;
    case ExpressionType::EXPRESSION_ZERO_LITERAL:
        borrow_check_zero_literal_expression(dynamic_cast<ZeroLiteralExpression *>(expression), ownership_table);
        break;
    case ExpressionType::EXPRESSION_FN:
        borrow_check_fn_expression(dynamic_cast<FnExpression *>(expression), ownership_table);
        break;
    case ExpressionType::EXPRESSION_SLICE:
        borrow_check_slice_expression(dynamic_cast<SliceExpression *>(expression), ownership_table);
        break;
    case ExpressionType::EXPRESSION_SUBSCRIPT:
        borrow_check_subscript_expression(dynamic_cast<SubscriptExpression *>(expression), ownership_table);
        break;
    default:
        panic("Cannot borrow check this expression :[");
    }
}

void BorrowChecker::borrow_check_is_expression(IsExpression *expression, OwnershipTable *ownership_table) {
}

void BorrowChecker::borrow_check_binary_expression(BinaryExpression *expression, OwnershipTable *ownership_table) {
}

void BorrowChecker::borrow_check_string_literal_expression(
    StringLiteralExpression *expression, OwnershipTable *ownership_table
) {
}

void BorrowChecker::borrow_check_bool_literal_expression(
    BoolLiteralExpression *expression, OwnershipTable *ownership_table
) {
}

void BorrowChecker::borrow_check_int_literal_expression(
    NumberLiteralExpression *expression, OwnershipTable *ownership_table
) {
}

void BorrowChecker::borrow_check_unary_expression(UnaryExpression *expression, OwnershipTable *ownership_table) {
}

void BorrowChecker::borrow_check_call_expression(CallExpression *expression, OwnershipTable *ownership_table) {
    for (auto arg : expression->args)
    { borrow_check_expression(arg, ownership_table); }
}

void BorrowChecker::borrow_check_identifier_expression(
    IdentifierExpression *expression, OwnershipTable *ownership_table
) {
}

void BorrowChecker::borrow_check_get_expression(GetExpression *expression, OwnershipTable *ownership_table) {
}

void BorrowChecker::borrow_check_instantiate_expression(
    InstantiateExpression *expression, OwnershipTable *ownership_table
) {
}

void BorrowChecker::borrow_check_group_expression(GroupExpression *expression, OwnershipTable *ownership_table) {
}

void BorrowChecker::borrow_check_null_literal_expression(
    NullLiteralExpression *expression, OwnershipTable *ownership_table
) {
}

void BorrowChecker::borrow_check_propagate_expression(
    PropagateExpression *expression, OwnershipTable *ownership_table
) {
}

void BorrowChecker::borrow_check_zero_literal_expression(
    ZeroLiteralExpression *expression, OwnershipTable *ownership_table
) {
}

void BorrowChecker::borrow_check_fn_expression(FnExpression *expression, OwnershipTable *ownership_table) {
}

void BorrowChecker::borrow_check_slice_expression(SliceExpression *expression, OwnershipTable *ownership_table) {
}

void BorrowChecker::borrow_check_subscript_expression(
    SubscriptExpression *expression, OwnershipTable *ownership_table
) {
}
