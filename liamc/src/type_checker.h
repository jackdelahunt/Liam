#pragma once
#include <map>

#include "parser.h"
#include "type_info.h"

struct Statement;
struct LetStatement;
struct ScopeStatement;
struct StructStatement;
struct FnStatement;
struct LoopStatement;
struct InsertStatement;
struct ReturnStatement;
struct BreakStatement;
struct ExpressionStatement;
struct AssigmentStatement;
struct IntLiteralExpression;
struct StringLiteralExpression;
struct Expression;
struct File;

struct SymbolTable {
    std::map<std::string, TypeInfo *> builtin_type_table; // u64 string...
    std::map<std::string, TypeInfo *> type_table;         // structs
    std::map<std::string, TypeInfo *> identifier_table;   // variables or funcs

    SymbolTable();

    void add_type(Token type, TypeInfo *type_info);
    void add_identifier(Token identifier, TypeInfo *type_info);
    TypeInfo *get_type(Token *identifier);
    TypeInfo *get_type(std::string identifier);
    SymbolTable copy();
};

struct TypeChecker {
    SymbolTable symbol_table;

    TypeChecker();

    File type_check(std::vector<File *> *files);

    void type_check_fn_decl(FnStatement *statement, SymbolTable *symbol_table);

    void type_check_statement(Statement *statement, SymbolTable *symbol_table);
    void type_check_insert_statement(InsertStatement *statement, SymbolTable *symbol_table);
    void type_check_return_statement(ReturnStatement *statement, SymbolTable *symbol_table);
    void type_check_break_statement(BreakStatement *statement, SymbolTable *symbol_table);
    void type_check_let_statement(LetStatement *statement, SymbolTable *symbol_table);
    void type_check_scope_statement(
        ScopeStatement *statement, SymbolTable *symbol_table, bool copy_symbol_table = true
    );
    void type_check_fn_statement(FnStatement *statement, SymbolTable *symbol_table, bool first_pass = false);
    void type_check_loop_statement(LoopStatement *statement, SymbolTable *symbol_table);
    void type_check_for_statement(ForStatement *statement, SymbolTable *symbol_table);
    void type_check_if_statement(IfStatement *statement, SymbolTable *symbol_table);
    void type_check_struct_statement(StructStatement *statement, SymbolTable *symbol_table, bool first_pass = false);
    void type_check_assigment_statement(AssigmentStatement *statement, SymbolTable *symbol_table);
    void type_check_expression_statement(ExpressionStatement *statement, SymbolTable *symbol_table);

    void type_check_expression(Expression *expression, SymbolTable *symbol_table);
    void type_check_identifier_expression(IdentifierExpression *expression, SymbolTable *symbol_table);
    void type_check_is_expression(IsExpression *expression, SymbolTable *symbol_table);
    void type_check_binary_expression(BinaryExpression *expression, SymbolTable *symbol_table);
    void type_check_string_literal_expression(StringLiteralExpression *expression, SymbolTable *symbol_table);
    void type_check_int_literal_expression(IntLiteralExpression *expression, SymbolTable *symbol_table);
    void type_check_bool_literal_expression(BoolLiteralExpression *expression, SymbolTable *symbol_table);
    void type_check_unary_expression(UnaryExpression *expression, SymbolTable *symbol_table);
    void type_check_call_expression(CallExpression *expression, SymbolTable *symbol_table);
    void type_check_get_expression(GetExpression *expression, SymbolTable *symbol_table);
    void type_check_new_expression(NewExpression *expression, SymbolTable *symbol_table);
    void type_check_group_expression(GroupExpression *expression, SymbolTable *symbol_table);

    void type_check_type_expression(TypeExpression *type_expression, SymbolTable *symbol_table);
    void type_check_union_type_expression(UnionTypeExpression *type_expression, SymbolTable *symbol_table);
    void type_check_unary_type_expression(UnaryTypeExpression *type_expression, SymbolTable *symbol_table);
    void type_check_specified_generics_type_expression(
        SpecifiedGenericsTypeExpression *type_expression, SymbolTable *symbol_table
    );
    void type_check_identifier_type_expression(IdentifierTypeExpression *type_expression, SymbolTable *symbol_table);

    TypeInfo *resolve_generics(TypeInfo *type_info, std::vector<TypeExpression *> *generic_params);
};

bool type_match(TypeInfo *a, TypeInfo *b);
