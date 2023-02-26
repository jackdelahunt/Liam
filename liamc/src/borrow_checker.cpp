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

void BorrowChecker::borrow_check(std::vector<File *> *files) {

    OwnershipTable empty = OwnershipTable();

    for (auto file : *files)
    {
        this->current_file = file;

        for (auto stmt : file->statements)
        { TRY_CALL(borrow_check_statement(stmt, &empty)); }
    }
}

void BorrowChecker::borrow_check_statement(Statement *statement, OwnershipTable *ownership_table) {
    switch (statement->statement_type)
    {
    case StatementType::STATEMENT_RETURN:
        return borrow_check_return_statement(dynamic_cast<ReturnStatement *>(statement), ownership_table);
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
    case StatementType::STATEMENT_TEST:
        if (args->test)
            return borrow_check_test_statement(dynamic_cast<TestStatement *>(statement), ownership_table);
        break;
    }
}

void BorrowChecker::borrow_check_return_statement(ReturnStatement *statement, OwnershipTable *ownership_table) {
    borrow_check_expression(statement->expression, ownership_table);
}

void BorrowChecker::borrow_check_let_statement(LetStatement *statement, OwnershipTable *ownership_table) {

    borrow_check_expression(statement->rhs, ownership_table);

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
    borrow_check_expression(statement->assigned_to->expression, ownership_table);
}

void BorrowChecker::borrow_check_expression_statement(ExpressionStatement *statement, OwnershipTable *ownership_table) {
    borrow_check_expression(statement->expression, ownership_table);
}

void BorrowChecker::borrow_check_for_statement(ForStatement *statement, OwnershipTable *ownership_table) {
    auto copy = ownership_table->copy();
    borrow_check_statement(statement->assign, &copy);
    borrow_check_expression(statement->condition, &copy);
    borrow_check_scope_statement(statement->body, &copy);

    // update is borrow checked last as this means we can then check for any
    // moves in the body so the update statement is always valid
    borrow_check_statement(statement->update, &copy);
}

void BorrowChecker::borrow_check_if_statement(IfStatement *statement, OwnershipTable *ownership_table) {
    borrow_check_expression(statement->expression, ownership_table);

    auto if_copy = ownership_table->copy();
    borrow_check_scope_statement(statement->body, &if_copy);

    if (statement->else_statement)
    {
        auto else_copy = ownership_table->copy();
        borrow_check_else_statement(statement->else_statement, &else_copy);
    }
}

void BorrowChecker::borrow_check_else_statement(ElseStatement *statement, OwnershipTable *ownership_table) {

    // make sure the else has a body or a if branch but not both
    ASSERT(statement->body || statement->if_statement);
    ASSERT(!(statement->body && statement->if_statement));

    if(statement->body) {
        borrow_check_scope_statement(statement->body, ownership_table);
    }

    if(statement->if_statement) {
        borrow_check_if_statement(statement->if_statement, ownership_table);
    }

}

void BorrowChecker::borrow_check_test_statement(TestStatement *statement, OwnershipTable *ownership_table) {
}

void BorrowChecker::borrow_check_expression(Expression *expression, OwnershipTable *ownership_table) {
    switch (expression->type)
    {
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
    case ExpressionType::EXPRESSION_PROPAGATE:
        borrow_check_propagate_expression(dynamic_cast<PropagateExpression *>(expression), ownership_table);
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
    }
}

void BorrowChecker::borrow_check_is_expression(IsExpression *expression, OwnershipTable *ownership_table) {
}

void BorrowChecker::borrow_check_binary_expression(BinaryExpression *expression, OwnershipTable *ownership_table) {
}

void BorrowChecker::borrow_check_unary_expression(UnaryExpression *expression, OwnershipTable *ownership_table) {

    // if we are try to use the pointer ops then in all cases where t is ^T
    // both @t and *t are valid so skip them, this makes the borrow checking
    // of the identifier need no context
    if (expression->op.type == TokenType::TOKEN_AT || expression->op.type == TokenType::TOKEN_STAR)
    {
        if (expression->expression->type != ExpressionType::EXPRESSION_IDENTIFIER)
        { borrow_check_expression(expression->expression, ownership_table); }
    }
}

void BorrowChecker::borrow_check_call_expression(CallExpression *expression, OwnershipTable *ownership_table) {
    for (auto arg : expression->args)
    { borrow_check_expression(arg, ownership_table); }
}

void BorrowChecker::borrow_check_identifier_expression(
    IdentifierExpression *expression, OwnershipTable *ownership_table
) {
    if (expression->type_info->type != TypeInfoType::OWNED_POINTER)
        return;

    auto identifier = expression->identifier.string;
    ASSERT(ownership_table->ownedValueExistedInScope(identifier));

    OwnershipStatus status = ownership_table->getOwnershipStatus(identifier);
    if (status.owner == OwnershipStatus::MOVED)
    {
        ErrorReporter::report_borrow_checker_error(
            this->current_file->path.string(), status.ownership_of_value, status.move_of_value, expression,
            "Use of already moved value"
        );
        return;
    }

    // now mark the value as moved
    status.move_of_value = expression;
    status.owner         = OwnershipStatus::MOVED;
    ownership_table->setOwnershipStatus(identifier, status);
}

void BorrowChecker::borrow_check_get_expression(GetExpression *expression, OwnershipTable *ownership_table) {
    if (expression->lhs->type_info->type != TypeInfoType::OWNED_POINTER)
        return;

    if (expression->lhs->type == ExpressionType::EXPRESSION_IDENTIFIER)
    {
        auto identifier = dynamic_cast<IdentifierExpression *>(expression->lhs);
        ASSERT(ownership_table->ownedValueExistedInScope(identifier->identifier.string));

        auto status = ownership_table->getOwnershipStatus(identifier->identifier.string);
        if (status.owner == OwnershipStatus::MOVED)
        {
            ErrorReporter::report_borrow_checker_error(
                this->current_file->path.string(), status.ownership_of_value, status.move_of_value, expression,
                "Trying to use members of an already moved value"
            );
            return;
        }
    }
}

void BorrowChecker::borrow_check_instantiate_expression(
    InstantiateExpression *expression, OwnershipTable *ownership_table
) {
}

void BorrowChecker::borrow_check_group_expression(GroupExpression *expression, OwnershipTable *ownership_table) {
    borrow_check_expression(expression->expression, ownership_table);
}

void BorrowChecker::borrow_check_propagate_expression(
    PropagateExpression *expression, OwnershipTable *ownership_table
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
