#include "emitter.h"
#include "statement.h"
#include "liam.h"

Emitter::Emitter() {
	local_max = 0;
	locals = std::map<std::string, int>();
}

std::string Emitter::emit(File& file) {
	auto source_generated = std::string();
	for (auto stmt : file.statements) {
		source_generated.append(emit_statement(stmt));
	}
	return source_generated;
}

std::string Emitter::emit_statement(Statement* statement) {
	{
		auto ptr = dynamic_cast<InsertStatement*>(statement);
		if (ptr) {
			return emit_insert_statement(ptr);
		}
	}

	{
		auto ptr = dynamic_cast<LetStatement*>(statement);
		if (ptr) {
			return emit_let_statement(ptr);
		}
	}

	{
		auto ptr = dynamic_cast<FnStatement*>(statement);
		if (ptr) {
			return emit_fn_statement(ptr);
		}
	}

	panic("Oh no");
}

std::string Emitter::emit_insert_statement(InsertStatement* statement) {
	return emit_expression(statement->byte_code);
}

std::string Emitter::emit_let_statement(LetStatement* statement) {
	auto expr = emit_expression(statement->expression);
	if (locals.contains(statement->identifier.string)) {
		panic("Already exists");
	}

	locals[statement->identifier.string] = local_max++;
	expr.append("\nstore " + std::to_string(local_max - 1) + "\n");
	return expr;
}

std::string Emitter::emit_fn_statement(FnStatement* statement) {
	auto fn_source = std::string();
	fn_source.append("@");
	fn_source.append(statement->identifier.string);
	fn_source.append("\n");
	for (auto stmt : statement->body) {
		fn_source.append(emit_statement(stmt));
	}
	return fn_source;
}

std::string Emitter::emit_expression(Expression* expression) {
	{
		auto ptr = dynamic_cast<StringLiteralExpression*>(expression);
		if (ptr) {
			return emit_string_literal_expression(ptr);
		}
	}

	{
		auto ptr = dynamic_cast<IntLiteralExpression*>(expression);
		if (ptr) {
			return emit_int_literal_expression(ptr);
		}
	}

	panic("How did we get here");
}

std::string Emitter::emit_int_literal_expression(IntLiteralExpression* expression) {
	return "push " + expression->token.string;
}

std::string Emitter::emit_string_literal_expression(StringLiteralExpression* expression) {
	return expression->token.string;
}