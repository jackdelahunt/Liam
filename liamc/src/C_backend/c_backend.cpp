#include "C_backend/c_backend.h"

#include "statement.h"
#include "liam.h"

#define hello 6;

CBackend::CBackend() {
}

std::string CBackend::emit(File& file) {
	auto source_generated = std::string("#include<stdio.h>\n#include \"string.h\"\n#define u64 int\n\n");
	for (auto stmt : file.statements) {
		source_generated.append(emit_statement(stmt));
	}

	return source_generated;
}

std::string CBackend::emit_statement(Statement* statement) {
	{
		auto ptr = dynamic_cast<InsertStatement*>(statement);
		if (ptr) {
			return emit_insert_statement(ptr);
		}
	}

	{
		auto ptr = dynamic_cast<ReturnStatement*>(statement);
		if (ptr) {
			return emit_return_statement(ptr);
		}
	}

	{
		auto ptr = dynamic_cast<BreakStatement*>(statement);
		if (ptr) {
			return emit_break_statement(ptr);
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
			auto s = emit_fn_statement(ptr);
			return s;
		}
	}

	{
		auto ptr = dynamic_cast<LoopStatement*>(statement);
		if (ptr) {
			auto s = emit_loop_statement(ptr);
			return s;
		}
	}

	{
		auto ptr = dynamic_cast<StructStatement*>(statement);
		if (ptr) {
			auto s = emit_struct_statement(ptr);
			return s;
		}
	}

	{
		auto ptr = dynamic_cast<AssigmentStatement*>(statement);
		if (ptr) {
			auto s = emit_assigment_statement(ptr);
			return s;
		}
	}

	{
		auto ptr = dynamic_cast<ExpressionStatement*>(statement);
		if (ptr) {
			return emit_expression_statement(ptr);
		}
	}

	panic("Oh no");
}

std::string CBackend::emit_insert_statement(InsertStatement* statement) {
	auto string_lit = dynamic_cast<StringLiteralExpression*>(statement->byte_code);
	if (string_lit) {
		return string_lit->token.string; // 
	}
	panic("Insert only accepts string literals");
	throw;	
}

std::string CBackend::emit_return_statement(ReturnStatement* statement) {
	auto expression = emit_expression(statement->expression);
	if (expression.empty())
		return "ret\n";

	expression.append("pop_ret\n");
	return expression;
}

std::string CBackend::emit_break_statement(BreakStatement* statement) {
	return "goto " + statement->identifier.string + ";\n";
}


std::string CBackend::emit_let_statement(LetStatement* statement) {
	auto source = std::string();
	source.append(emit_expression(statement->type) + " ");
	source.append(statement->identifier.string + " = ");
	auto emitted_expr = emit_expression(statement->expression);
	source.append(emitted_expr + ";\n");
	return source;
}

std::string CBackend::emit_scope_statement(ScopeStatement* statement) {
	auto fn_source = std::string("{\n");
	for (auto stmt : statement->body) {
		fn_source.append(emit_statement(stmt));
	}
	fn_source.append("\n}\n");
	return fn_source;
}

std::string CBackend::emit_fn_statement(FnStatement* statement) {
	auto fn_source = std::string();
	fn_source.append(emit_expression(statement->type) + " ");
	fn_source.append(statement->identifier.string);
	fn_source.append("(");	
	
	int index = 0;
	for (auto& [identifier, type] : statement->params) {
		fn_source.append(emit_expression(type) + " " + identifier.string);
		index++;
		if (index < statement->params.size()) {
			fn_source.append(", ");
		}
	}
	fn_source.append(")");


	fn_source.append(emit_scope_statement(statement->body));

	return fn_source + "\n";
}

std::string CBackend::emit_loop_statement(LoopStatement* statement) {
	auto source = std::string();
	source.append("while(true)");
	source.append(emit_scope_statement(statement->body));
	source.append(statement->identifier.string + ": ;\n");
	return source;
}

std::string CBackend::emit_struct_statement(StructStatement* statement) {
	auto source = std::string();
	source.append("struct " + statement->identifier.string + "{\n");
	for (auto& [identifier, type] : statement->members) {
		source.append(emit_expression(type) + " " + identifier.string + ";\n");
	}
	source.append("\n};\n\n");
	return source;
}

std::string CBackend::emit_assigment_statement(AssigmentStatement* statement) {
	auto source = std::string();
	source.append(statement->identifier.string + " = ");
	source.append(emit_expression_statement(statement->assigned_to));
	return source;
}

std::string CBackend::emit_expression_statement(ExpressionStatement* statement) {
	return emit_expression(statement->expression) + ";\n";
}

std::string CBackend::emit_expression(Expression* expression) {
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

	{
		auto ptr = dynamic_cast<CallExpression*>(expression);
		if (ptr) {
			return emit_call_expression(ptr);
		}
	}

	{
		auto ptr = dynamic_cast<IdentifierExpression*>(expression);
		if (ptr) {
			return emit_identifier_expression(ptr);
		}
	}

	{
		auto ptr = dynamic_cast<BinaryExpression*>(expression);
		if (ptr) {
			return emit_binary_expression(ptr);
		}
	}

	{
		auto ptr = dynamic_cast<UnaryExpression*>(expression);
		if (ptr) {
			return emit_unary_expression(ptr);
		}
	}

	{
		auto ptr = dynamic_cast<TypeLiteralExpression*>(expression);
		if (ptr) {
			return emit_type_literal_expression(ptr);
		}
	}

	{
		auto ptr = dynamic_cast<GetExpression*>(expression);
		if (ptr) {
			return emit_get_expression(ptr);
		}
	}

	{
		auto ptr = dynamic_cast<NewExpression*>(expression);
		if (ptr) {
			return emit_new_expression(ptr);
		}
	}

	// given empty expression
	return "";
}

std::string CBackend::emit_binary_expression(BinaryExpression* expression) {
	auto source = std::string();
	source.append(emit_expression(expression->left));
	switch (expression->op.type)
	{
	case TOKEN_PLUS:
		source.append(" + ");
		break;
	default:
		panic("Cannot use this operand");
		break;
	}
	source.append(emit_expression(expression->right));

	return source;
}

std::string CBackend::emit_int_literal_expression(IntLiteralExpression* expression) {
	return expression->token.string;
}

std::string CBackend::emit_string_literal_expression(StringLiteralExpression* expression) {
	return "\"" + expression->token.string + "\"";
}

std::string CBackend::emit_call_expression(CallExpression* expression) {
	auto source = std::string();
	source.append(dynamic_cast<IdentifierExpression*>(expression->identifier)->identifier.string + "(");
	int index = 0;
	for (auto expr : expression->args) {
		source.append(emit_expression(expr));
		index++;
		if (index < expression->args.size()) {
			source.append(", ");
		}
	}
	source.append(")");

	return source;
}

std::string CBackend::emit_unary_expression(UnaryExpression* expression) {
	// int x = 10;
	// int* a = &x;
	// int b = *a;
	
	// let x: u64 = 10;
	// let a: u64^ = @x;
	// let b: u64 = *a;

	if (expression->op.type == TOKEN_HAT) {
		return emit_expression(expression->expression) + "*";
	}
	else if (expression->op.type == TOKEN_AT) {
		return "&" + emit_expression(expression->expression);
	}
	else if (expression->op.type == TOKEN_STAR) {
		return "*" + emit_expression(expression->expression);
	}

	panic("Got a unrecognized operand");
}

std::string CBackend::emit_identifier_expression(IdentifierExpression* expression) {
	return expression->identifier.string;
}

std::string CBackend::emit_type_literal_expression(TypeLiteralExpression* expression) {
	return expression->token.string;
}

std::string CBackend::emit_get_expression(GetExpression* expression) {
	return emit_expression(expression->expression) + "." + expression->member.string;
}

std::string CBackend::emit_new_expression(NewExpression* expression) {
	std::string source = "{";
	int index = 0;
	for (auto expr : expression->expressions) {
		source.append(emit_expression(expr));
		index++;
		if (index < expression->expressions.size()) {
			source.append(", ");
		}
	}

	source.append("}");

	return source;
}