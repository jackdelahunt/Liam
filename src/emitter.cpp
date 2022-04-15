#include "emitter.h"
#include "statement.h"
#include "liam.h"

Scope::Scope() {
	locals = std::map<std::string, int>();
	local_max = 0;
}

Emitter::Emitter() {
}

std::string Emitter::emit(File& file) {
	auto source_generated = std::string();
	for (auto stmt : file.statements) {
		auto scope = Scope();
		source_generated.append(emit_statement(stmt, &scope));
	}
	return source_generated;
}

std::string Emitter::emit_statement(Statement* statement, Scope* scope) {
	{
		auto ptr = dynamic_cast<InsertStatement*>(statement);
		if (ptr) {
			return emit_insert_statement(ptr, scope);
		}
	}

	{
		auto ptr = dynamic_cast<ReturnStatement*>(statement);
		if (ptr) {
			return emit_return_statement(ptr, scope);
		}
	}

	{
		auto ptr = dynamic_cast<LetStatement*>(statement);
		if (ptr) {
			return emit_let_statement(ptr, scope);
		}
	}

	{
		auto ptr = dynamic_cast<FnStatement*>(statement);
		if (ptr) {
			return emit_fn_statement(ptr, scope);
		}
	}

	{
		auto ptr = dynamic_cast<ExpressionStatement*>(statement);
		if (ptr) {
			return emit_expression_statement(ptr, scope);
		}
	}

	panic("Oh no");
}

std::string Emitter::emit_insert_statement(InsertStatement* statement, Scope* scope) {
	return emit_expression(statement->byte_code, scope);
}

std::string Emitter::emit_return_statement(ReturnStatement* statement, Scope* scope) {
	return "ret\n";
}

std::string Emitter::emit_let_statement(LetStatement* statement, Scope* scope) {
	auto expr = emit_expression(statement->expression, scope);
	if (scope->locals.contains(statement->identifier.string)) {
		panic("Already exists");
	}

	scope->locals[statement->identifier.string] = scope->local_max++;
	expr.append("store " + std::to_string(scope->local_max - 1) + "\n");
	return expr;
}

std::string Emitter::emit_fn_statement(FnStatement* statement, Scope* scope) {
	auto fn_source = std::string();
	fn_source.append("@");
	fn_source.append(statement->identifier.string);
	fn_source.append("\n");
	for (auto stmt : statement->body) {
		auto sub_scope = Scope();

		// setup locals for sub scope
		for (auto& param : statement->params) {
			sub_scope.locals[param.string] = sub_scope.local_max++;
		}

		fn_source.append(emit_statement(stmt, &sub_scope));
	}
	return fn_source + "\n";
}

std::string Emitter::emit_expression_statement(ExpressionStatement* statement, Scope* scope) {
	return emit_expression(statement->expression, scope);
}

std::string Emitter::emit_expression(Expression* expression, Scope* scope) {
	{
		auto ptr = dynamic_cast<StringLiteralExpression*>(expression);
		if (ptr) {
			return emit_string_literal_expression(ptr, scope);
		}
	}

	{
		auto ptr = dynamic_cast<IntLiteralExpression*>(expression);
		if (ptr) {
			return emit_int_literal_expression(ptr, scope);
		}
	}

	{
		auto ptr = dynamic_cast<CallExpression*>(expression);
		if (ptr) {
			return emit_call_expression(ptr, scope);
		}
	}

	{
		auto ptr = dynamic_cast<IdentifierExpression*>(expression);
		if (ptr) {
			return emit_identifier_expression(ptr, scope); // temp
		}
	}

	panic("How did we get here");
}

std::string Emitter::emit_int_literal_expression(IntLiteralExpression* expression, Scope* scope) {
	return "push " + expression->token.string + "\n";
}

std::string Emitter::emit_string_literal_expression(StringLiteralExpression* expression, Scope* scope) {
	return expression->token.string + "\n";
}

std::string Emitter::emit_call_expression(CallExpression* expression, Scope* scope) {
	auto source = std::string();
	for (auto expr : expression->args) {
		source.append(emit_expression(expr, scope));
	}

	// TODO: very stinky
	auto identifier = dynamic_cast<IdentifierExpression*>(expression->identifier)->identifier.string;
	source.append("call @" + identifier + " " + std::to_string(expression->args.size()) + "\n");
	return source;
}

std::string Emitter::emit_identifier_expression(IdentifierExpression* expression, Scope* scope) {
	std::string source = "load ";
	source.append(std::to_string(scope->locals[expression->identifier.string]) + "\n");
	return source;
}