#pragma once
#include <map>
#include <string>

#include "expression.h"
#include "parser.h"
#include "statement.h"
#include "type_checker.h"

struct CppBackend {
    Module *current_module = NULL;
    File *current_file     = NULL;
    std::vector<Module *> *modules;
    CppBackend() = default;
    std::string emit(std::vector<Module *> *modules);

    std::string forward_declare_enum(EnumStatement *statement);
    std::string forward_declare_struct(StructStatement *statement);
    std::string forward_declare_function(FnStatement *statement);

    std::string emit_statement(Statement *statement);
    std::string emit_return_statement(ReturnStatement *statement);
    std::string emit_break_statement(BreakStatement *statement);
    std::string emit_let_statement(LetStatement *statement);
    std::string emit_scope_statement(ScopeStatement *statement);
    std::string emit_fn_statement(FnStatement *statement);
    std::string emit_struct_statement(StructStatement *statement);
    std::string emit_assigment_statement(AssigmentStatement *statement);
    std::string emit_expression_statement(ExpressionStatement *statement);
    std::string emit_for_statement(ForStatement *statement);
    std::string emit_if_statement(IfStatement *statement);
    std::string emit_else_statement(ElseStatement *statement);
    std::string emit_enum_statement(EnumStatement *statement);
    std::string emit_continue_statement(ContinueStatement *statement);

    std::string emit_expression(Expression *expression);
    std::string emit_binary_expression(BinaryExpression *expression);
    std::string emit_string_literal_expression(StringLiteralExpression *expression);
    std::string emit_bool_literal_expression(BoolLiteralExpression *expression);
    std::string emit_int_literal_expression(NumberLiteralExpression *expression);
    std::string emit_unary_expression(UnaryExpression *expression);
    std::string emit_subscript_expression(SubscriptExpression *expression);
    std::string emit_call_expression(CallExpression *expression);
    std::string emit_identifier_expression(IdentifierExpression *expression);
    std::string emit_get_expression(GetExpression *expression);
    std::string emit_group_expression(GroupExpression *expression);
    std::string emit_null_literal_expression(NullLiteralExpression *expression);
    std::string emit_zero_literal_expression(ZeroLiteralExpression *expression);
    std::string emit_fn_expression(FnExpression *expression);
    std::string emit_slice_literal_expression(SliceLiteralExpression *expression);
    std::string emit_instantiate_expression(InstantiateExpression *expression);
    std::string emit_enum_instance_expression(EnumInstanceExpression *expression);
    std::string emit_struct_instance_expression(StructInstanceExpression *expression);

    std::string emit_type_expression(TypeExpression *type_expression);
    std::string emit_unary_type_expression(UnaryTypeExpression *type_expression);
    std::string emit_specified_generics_type_expression(SpecifiedGenericsTypeExpression *type_expression);
    std::string emit_fn_type_expression(FnTypeExpression *type_expression);
    std::string emit_identifier_type_expression(IdentifierTypeExpression *type_expression);

    std::string emit_cpp_template_declaration(std::vector<Token> *generics);
    std::string emit_cpp_template_usage(std::vector<Token> *generics);
    std::string emit_cpp_template_params(std::vector<TypeExpression *> *generics);
};

std::string strip_semi_colon(std::string str);
u64 string_literal_length(std::string *string);
