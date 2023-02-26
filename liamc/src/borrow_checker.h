#pragma once

#include <map>
#include <string>

#include "expression.h"
#include "parser.h"
#include "statement.h"
#include "type_checker.h"

struct OwnershipStatus {

    enum Owner {
        SCOPE,
        MOVED
    } owner;

    TypeInfo *type_info;

    // cpp is so cringe, this is needed for std::map to work... why?? I dunno
    OwnershipStatus() = default;
    OwnershipStatus(Owner owner, TypeInfo *type_info);
};

struct OwnershipTable {
    std::map<std::string, OwnershipStatus> table;

    OwnershipTable();
    OwnershipTable copy();
    void addOwnedValue(std::string identifier, OwnershipStatus status);
    bool ownedValueExistedInScope(std::string identifier);
    OwnershipStatus getOwnershipStatus(std::string identifier);
    void setOwnershipStatus(std::string identifier, OwnershipStatus status);
};

struct BorrowChecker {

    void borrow_check(File *file);

    void borrow_check_statement(Statement *statement, OwnershipTable *ownership_table);
    void borrow_check_return_statement(ReturnStatement *statement, OwnershipTable *ownership_table);
    void borrow_check_break_statement(BreakStatement *statement, OwnershipTable *ownership_table);
    void borrow_check_let_statement(LetStatement *statement, OwnershipTable *ownership_table);
    void borrow_check_scope_statement(ScopeStatement *statement, OwnershipTable *ownership_table);
    void borrow_check_fn_statement(FnStatement *statement, OwnershipTable *ownership_table);
    void borrow_check_struct_statement(StructStatement *statement, OwnershipTable *ownership_table);
    void borrow_check_assigment_statement(AssigmentStatement *statement, OwnershipTable *ownership_table);
    void borrow_check_expression_statement(ExpressionStatement *statement, OwnershipTable *ownership_table);
    void borrow_check_for_statement(ForStatement *statement, OwnershipTable *ownership_table);
    void borrow_check_if_statement(IfStatement *statement, OwnershipTable *ownership_table);
    void borrow_check_else_statement(ElseStatement *statement, OwnershipTable *ownership_table);
    void borrow_check_enum_statement(EnumStatement *statement, OwnershipTable *ownership_table);
    void borrow_check_continue_statement(ContinueStatement *statement, OwnershipTable *ownership_table);
    void borrow_check_alias_statement(AliasStatement *statement, OwnershipTable *ownership_table);
    void borrow_check_test_statement(TestStatement *statement, OwnershipTable *ownership_table);

    void borrow_check_expression(Expression *expression, OwnershipTable *ownership_table);
    void borrow_check_is_expression(IsExpression *expression, OwnershipTable *ownership_table);
    void borrow_check_binary_expression(BinaryExpression *expression, OwnershipTable *ownership_table);
    void borrow_check_string_literal_expression(StringLiteralExpression *expression, OwnershipTable *ownership_table);
    void borrow_check_bool_literal_expression(BoolLiteralExpression *expression, OwnershipTable *ownership_table);
    void borrow_check_int_literal_expression(NumberLiteralExpression *expression, OwnershipTable *ownership_table);
    void borrow_check_unary_expression(UnaryExpression *expression, OwnershipTable *ownership_table);
    void borrow_check_call_expression(CallExpression *expression, OwnershipTable *ownership_table);
    void borrow_check_identifier_expression(IdentifierExpression *expression, OwnershipTable *ownership_table);
    void borrow_check_get_expression(GetExpression *expression, OwnershipTable *ownership_table);
    void borrow_check_instantiate_expression(InstantiateExpression *expression, OwnershipTable *ownership_table);
    void borrow_check_group_expression(GroupExpression *expression, OwnershipTable *ownership_table);
    void borrow_check_null_literal_expression(NullLiteralExpression *expression, OwnershipTable *ownership_table);
    void borrow_check_propagate_expression(PropagateExpression *expression, OwnershipTable *ownership_table);
    void borrow_check_zero_literal_expression(ZeroLiteralExpression *expression, OwnershipTable *ownership_table);
    void borrow_check_fn_expression(FnExpression *expression, OwnershipTable *ownership_table);
    void borrow_check_slice_expression(SliceExpression *expression, OwnershipTable *ownership_table);
    void borrow_check_subscript_expression(SubscriptExpression *expression, OwnershipTable *ownership_table);
};