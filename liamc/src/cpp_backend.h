#pragma once
#include <string>

#include "ast.h"
#include "parser.h"
#include "type_checker.h"

// If defined, the CppBuiler will also print to stdout
// as it is building, this is useful for stepping through
// with a debugger and seeing the real time output
// #define PRINT_CPP_BUILDER

struct CppBuilder {
    std::string source;
    u64 indentation;

    CppBuilder();

    void start_line();
    void end_line();
    void append(std::string string);
    void append_line(std::string string);
    void insert_new_line();
    void append_indentation();
    void indent();
    void un_indent();
};

struct CppBackend {
    CompilationUnit *compilation_unit;
    CompilationBundle *compilation_bundle;
    CppBuilder builder;

    CppBackend();

    std::string emit(CompilationBundle *bundle);

    void forward_declare_namespace(CompilationUnit *compilation_unit);
    void forward_declare_struct(StructStatement *statement);
    void forward_declare_function(FnStatement *statement);

    void emit_statement(Statement *statement);
    void emit_import_statement(ImportStatement *statement);
    void emit_return_statement(ReturnStatement *statement);
    void emit_break_statement(BreakStatement *statement);
    void emit_let_statement(LetStatement *statement);
    void emit_scope_statement(ScopeStatement *statement);
    void emit_fn_statement(FnStatement *statement);
    void emit_struct_statement(StructStatement *statement);
    void emit_assigment_statement(AssigmentStatement *statement);
    void emit_expression_statement(ExpressionStatement *statement);
    void emit_for_statement(ForStatement *statement);
    void emit_if_statement(IfStatement *statement);
    void emit_else_statement(ElseStatement *statement);
    void emit_continue_statement(ContinueStatement *statement);
    void emit_print_statement(PrintStatement *statement);
    void emit_assert_statement(AssertStatement *statement);

    void emit_expression(Expression *expression);
    void emit_binary_expression(BinaryExpression *expression);
    void emit_string_literal_expression(StringLiteralExpression *expression);
    void emit_bool_literal_expression(BoolLiteralExpression *expression);
    void emit_int_literal_expression(NumberLiteralExpression *expression);
    void emit_unary_expression(UnaryExpression *expression);
    void emit_call_expression(CallExpression *expression);
    void emit_identifier_expression(IdentifierExpression *expression);
    void emit_get_expression(GetExpression *expression);
    void emit_group_expression(GroupExpression *expression);
    void emit_null_literal_expression(NullLiteralExpression *expression);
    void emit_zero_literal_expression(ZeroLiteralExpression *expression);
    void emit_instantiate_expression(InstantiateExpression *expression);
    void emit_struct_instance_expression(StructInstanceExpression *expression);
    void emit_static_array_literal_expression(StaticArrayExpression *expression);
    void emit_subscript_expression(SubscriptExpression *expression);
    void emit_range_slicing_expression(RangeExpression *expression);

    void emit_type_expression(TypeExpression *type_expression);
    void emit_unary_type_expression(UnaryTypeExpression *type_expression);
    void emit_identifier_type_expression(IdentifierTypeExpression *type_expression);
    void emit_get_type_expression(GetTypeExpression *type_expression);
    void emit_static_array_type_expression(StaticArrayTypeExpression *type_expression);
    void emit_slice_type_expression(SliceTypeExpression *type_expression);
};

std::string strip_semi_colon(std::string str);
u64 string_literal_length(std::string *string);
std::string get_namespace_name(CompilationUnit *compilation_unit);
