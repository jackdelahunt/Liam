#pragma once
#include <list>
#include <map>

#include "parser.h"

struct Statement;
struct LetStatement;
struct ScopeStatement;
struct StructStatement;
struct FnStatement;
struct ReturnStatement;
struct BreakStatement;
struct ExpressionStatement;
struct AssigmentStatement;
struct NumberLiteralExpression;
struct StringLiteralExpression;
struct Expression;
struct CompilationUnit;

struct TopLevelDescriptor {
    std::string identifier;
    TypeInfo *type_info;
};

typedef std::unordered_map<std::string, TypeInfo *> Scope;

struct TypeChecker {
    CompilationUnit *compilation_unit;

    Scope global_type_scope;
    Scope *global_fn_scope; // pointer as it is the bottom scope in the scope list
    std::list<Scope> scopes;

    TypeChecker();

    void new_scope();
    void delete_scope();
    void add_type_to_scope(TokenIndex token_index, TypeInfo *type_info);
    TypeInfo *get_type_from_scope(TokenIndex token_index);
    void add_to_scope(TokenIndex token_index, TypeInfo *type_info);
    TypeInfo *get_from_scope(TokenIndex token_index);

    void print_fn_scope();

    void type_check(CompilationUnit *file);

    void type_check_fn_symbol(FnStatement *statement);
    void type_check_struct_symbol(StructStatement *statement);
    void type_check_fn_decl(FnStatement *statement);
    void type_check_fn_statement_full(FnStatement *statement);
    void type_check_struct_statement_full(StructStatement *statement);

    void type_check_statement(Statement *statement);
    void type_check_return_statement(ReturnStatement *statement);
    void type_check_break_statement(BreakStatement *statement);
    void type_check_let_statement(LetStatement *statement);
    void type_check_scope_statement(ScopeStatement *statement, bool copy_symbol_table = true);
    void type_check_for_statement(ForStatement *statement);
    void type_check_if_statement(IfStatement *statement);
    void type_check_else_statement(ElseStatement *statement);
    void type_check_assigment_statement(AssigmentStatement *statement);
    void type_check_expression_statement(ExpressionStatement *statement);

    void type_check_expression(Expression *expression);
    void type_check_identifier_expression(IdentifierExpression *expression);
    void type_check_binary_expression(BinaryExpression *expression);
    void type_check_string_literal_expression(StringLiteralExpression *expression);
    void type_check_number_literal_expression(NumberLiteralExpression *expression);
    void type_check_bool_literal_expression(BoolLiteralExpression *expression);
    void type_check_unary_expression(UnaryExpression *expression);
    void type_check_call_expression(CallExpression *expression);
    void type_check_fn_call_expression(CallExpression *expression);
    void type_check_get_expression(GetExpression *expression);
    void type_check_group_expression(GroupExpression *expression);
    void type_check_null_literal_expression(NullLiteralExpression *expression);
    void type_check_zero_literal_expression(ZeroLiteralExpression *expression);
    void type_check_instantiate_expression(InstantiateExpression *expression);
    void type_check_struct_instance_expression(StructInstanceExpression *expression);

    void type_check_type_expression(TypeExpression *type_expression);
    void type_check_unary_type_expression(UnaryTypeExpression *type_expression);
    void type_check_identifier_type_expression(IdentifierTypeExpression *type_expression);
};

bool type_match(TypeInfo *a, TypeInfo *b);
StructTypeInfo *get_struct_type_info_from_type_info(TypeInfo *type_info);
