#pragma once
#include <string>
#include <map>
#include "parser.h"
#include "statement.h"
#include "expression.h"
#include "type_checker.h"

struct RustBackend {
	RustBackend() = default;
	std::string emit(TypedFile& file);
	std::string emit_statement(TypedStatement* statement);
	std::string emit_insert_statement(TypedInsertStatement* statement);
	std::string emit_return_statement(TypedReturnStatement* statement);
	std::string emit_break_statement(TypedBreakStatement* statement);
	std::string emit_let_statement(TypedLetStatement* statement);
	std::string emit_scope_statement(TypedScopeStatement* statement);
	std::string emit_fn_statement(TypedFnStatement* statement);
	std::string emit_loop_statement(TypedLoopStatement* statement);
	std::string emit_struct_statement(TypedStructStatement* statement);
	std::string emit_assigment_statement(TypedAssigmentStatement* statement);
	std::string emit_expression_statement(TypedExpressionStatement* statement);
    std::string emit_expression(TypedExpression* expression);
    std::string emit_cloneable_expression(TypedExpression* expression);
	std::string emit_binary_expression(TypedBinaryExpression* expression);
	std::string emit_string_literal_expression(TypedStringLiteralExpression* expression);
	std::string emit_int_literal_expression(TypedIntLiteralExpression* expression);
	std::string emit_unary_expression(TypedUnaryExpression* expression);
	std::string emit_call_expression(TypedCallExpression* expression);
	std::string emit_identifier_expression(TypedIdentifierExpression* expression);
	std::string emit_get_expression(TypedGetExpression* expression);
	std::string emit_new_expression(TypedNewExpression* expression);
};