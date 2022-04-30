#pragma once
#include <string>
#include <map>
#include "parser.h"
#include "statement.h"
#include "expression.h"

struct CBackend {
	CBackend();
	std::string emit(File& file);
	std::string emit_statement(Statement* statement);
	std::string emit_insert_statement(InsertStatement* statement);
	std::string emit_return_statement(ReturnStatement* statement);
	std::string emit_break_statement(BreakStatement* statement);
	std::string emit_let_statement(LetStatement* statement);
	std::string emit_scope_statement(ScopeStatement* statement);
	std::string emit_fn_statement(FnStatement* statement);
	std::string emit_loop_statement(LoopStatement* statement);
	std::string emit_struct_statement(StructStatement* statement);
	std::string emit_assigment_statement(AssigmentStatement* statement);
	std::string emit_expression_statement(ExpressionStatement* statement);
	std::string emit_expression(Expression* expression);
	std::string emit_binary_expression(BinaryExpression* expression);
	std::string emit_string_literal_expression(StringLiteralExpression* expression);
	std::string emit_int_literal_expression(IntLiteralExpression* expression);
	std::string emit_unary_expression(UnaryExpression* expression);
	std::string emit_call_expression(CallExpression* expression);
	std::string emit_identifier_expression(IdentifierExpression* expression);
	std::string emit_get_expression(GetExpression* expression);
	std::string emit_new_expression(NewExpression* expression);
};