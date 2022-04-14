#pragma once

#include <string>
#include <map>
#include "parser.h"
#include "statement.h"
#include "expression.h"

struct Emitter {
	std::map<std::string, int> locals;
	int local_max;

	Emitter();
	std::string emit(File& file);
	std::string emit_statement(Statement* statement);
	std::string emit_insert_statement(InsertStatement* statement);
	std::string emit_let_statement(LetStatement* statement);
	std::string emit_fn_statement(FnStatement* statement);
	std::string emit_expression(Expression* expression);
	std::string emit_string_literal_expression(StringLiteralExpression* expression);
	std::string emit_int_literal_expression(IntLiteralExpression* expression);
};