#pragma once

#include <string>
#include <map>
#include "parser.h"
#include "statement.h"
#include "expression.h"

struct Scope {
	std::map<std::string, int> locals;
	int local_max;

	Scope();
};

struct Emitter {

	Emitter();
	std::string emit(File& file);
	std::string emit_statement(Statement* statement, Scope* scope);
	std::string emit_insert_statement(InsertStatement* statement, Scope* scope);
	std::string emit_return_statement(ReturnStatement* statement, Scope* scope);
	std::string emit_let_statement(LetStatement* statement, Scope* scope);
	std::string emit_fn_statement(FnStatement* statement, Scope* scope);
	std::string emit_expression_statement(ExpressionStatement* statement, Scope* scope);
	std::string emit_expression(Expression* expression, Scope* scope);
	std::string emit_string_literal_expression(StringLiteralExpression* expression, Scope* scope);
	std::string emit_int_literal_expression(IntLiteralExpression* expression, Scope* scope);
	std::string emit_call_expression(CallExpression* expression, Scope* scope);
	std::string emit_identifier_expression(IdentifierExpression* expression, Scope* scope);
	std::string emit_binary_expression(BinaryExpression* expression, Scope* scope);
};

void undeclared_identidier(Token* token);