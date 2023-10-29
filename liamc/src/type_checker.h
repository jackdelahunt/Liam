#pragma once
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

struct SymbolTable {
    CompilationUnit *compilation_unit = NULL;

    std::unordered_map<std::string, TypeInfo *> local_generic_type_table; // generic type added into scope
    std::unordered_map<std::string, TypeInfo *> identifier_table;         // variables or funcs

    SymbolTable(CompilationUnit *current_file);
    SymbolTable() = default; // removing this makes a compile error... why? lululul

    void add_identifier_type(std::string identifier, TypeInfo *type_info);
    std::tuple<TypeInfo *, bool> get_identifier_type(std::string identifier);
    void add_compiler_generated_identifier(std::string identifier, TypeInfo *type_info);
    SymbolTable copy();
};

struct TypeChecker {
    CompilationUnit *compilation_unit;

    std::unordered_map<std::string, TypeInfo *> builtin_type_table;
    std::unordered_map<std::string, TopLevelDescriptor> top_level_type_table;
    std::unordered_map<std::string, TopLevelDescriptor> top_level_function_table;

    TypeChecker();

    void add_type(CompilationUnit *file, TokenIndex identifier, TypeInfo *type_info);
    void add_function(CompilationUnit *file, TokenIndex identifier, TypeInfo *type_info);
    std::tuple<TypeInfo *, bool> get_type(std::string identifier);
    std::tuple<TypeInfo *, bool> get_function(std::string identifier);

    void type_check(CompilationUnit *file);

    void type_check_fn_symbol(FnStatement *statement);
    void type_check_struct_symbol(StructStatement *statement);
    void type_check_fn_decl(FnStatement *statement);
    void type_check_fn_statement_full(FnStatement *statement);
    void type_check_struct_statement_full(StructStatement *statement);

    void type_check_statement(Statement *statement, SymbolTable *symbol_table);
    void type_check_return_statement(ReturnStatement *statement, SymbolTable *symbol_table);
    void type_check_break_statement(BreakStatement *statement, SymbolTable *symbol_table);
    void type_check_let_statement(LetStatement *statement, SymbolTable *symbol_table);
    void type_check_scope_statement(
        ScopeStatement *statement, SymbolTable *symbol_table, bool copy_symbol_table = true
    );
    void type_check_for_statement(ForStatement *statement, SymbolTable *symbol_table);
    void type_check_if_statement(IfStatement *statement, SymbolTable *symbol_table);
    void type_check_else_statement(ElseStatement *statement, SymbolTable *symbol_table);
    void type_check_assigment_statement(AssigmentStatement *statement, SymbolTable *symbol_table);
    void type_check_expression_statement(ExpressionStatement *statement, SymbolTable *symbol_table);

    void type_check_expression(Expression *expression, SymbolTable *symbol_table);
    void type_check_identifier_expression(IdentifierExpression *expression, SymbolTable *symbol_table);
    void type_check_binary_expression(BinaryExpression *expression, SymbolTable *symbol_table);
    void type_check_string_literal_expression(StringLiteralExpression *expression, SymbolTable *symbol_table);
    void type_check_number_literal_expression(NumberLiteralExpression *expression, SymbolTable *symbol_table);
    void type_check_bool_literal_expression(BoolLiteralExpression *expression, SymbolTable *symbol_table);
    void type_check_unary_expression(UnaryExpression *expression, SymbolTable *symbol_table);
    void type_check_subscript_expression(SubscriptExpression *expression, SymbolTable *symbol_table);
    void type_check_call_expression(CallExpression *expression, SymbolTable *symbol_table);
    void type_check_fn_call_expression(CallExpression *expression, SymbolTable *symbol_table);
    void type_check_fn_expression_call_expression(CallExpression *expression, SymbolTable *symbol_table);
    void type_check_get_expression(GetExpression *expression, SymbolTable *symbol_table);
    void type_check_group_expression(GroupExpression *expression, SymbolTable *symbol_table);
    void type_check_null_literal_expression(NullLiteralExpression *expression, SymbolTable *symbol_table);
    void type_check_zero_literal_expression(ZeroLiteralExpression *expression, SymbolTable *symbol_table);
    void type_check_fn_expression(FnExpression *expression, SymbolTable *symbol_table);
    void type_check_instantiate_expression(InstantiateExpression *expression, SymbolTable *symbol_table);
    void type_check_struct_instance_expression(StructInstanceExpression *expression, SymbolTable *symbol_table);

    void type_check_type_expression(TypeExpression *type_expression, SymbolTable *symbol_table);
    void type_check_unary_type_expression(UnaryTypeExpression *type_expression, SymbolTable *symbol_table);
    void type_check_fn_type_expression(FnTypeExpression *type_expression, SymbolTable *symbol_table);
    void type_check_identifier_type_expression(IdentifierTypeExpression *type_expression, SymbolTable *symbol_table);
};

bool type_match(TypeInfo *a, TypeInfo *b);
StructTypeInfo *get_struct_type_info_from_type_info(TypeInfo *type_info);
