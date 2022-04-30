#include "C_backend/c_backend.h"
#include "liam.h"

CBackend::CBackend() {
}

std::string CBackend::emit(TypedFile* file) {
	auto source_generated = std::string("#include<stdio.h>\n#include \"string.h\"\n#define u64 int\n\n");
	for (auto stmt : file->statements) {
		source_generated.append(emit_statement(stmt));
	}
    
	return source_generated;
}

std::string CBackend::emit_statement(TypedStatement* statement) {
	{
		auto ptr = dynamic_cast<TypedInsertStatement*>(statement);
		if (ptr) {
			return emit_insert_statement(ptr);
		}
	}
    
	{
		auto ptr = dynamic_cast<TypedReturnStatement*>(statement);
		if (ptr) {
			return emit_return_statement(ptr);
		}
	}
    
	{
		auto ptr = dynamic_cast<TypedBreakStatement*>(statement);
		if (ptr) {
			return emit_break_statement(ptr);
		}
	}
    
	{
		auto ptr = dynamic_cast<TypedLetStatement*>(statement);
		if (ptr) {
			return emit_let_statement(ptr);
		}
	}
    
	{
		auto ptr = dynamic_cast<TypedFnStatement*>(statement);
		if (ptr) {
			auto s = emit_fn_statement(ptr);
			return s;
		}
	}
    
	{
		auto ptr = dynamic_cast<TypedLoopStatement*>(statement);
		if (ptr) {
			auto s = emit_loop_statement(ptr);
			return s;
		}
	}
    
	{
		auto ptr = dynamic_cast<TypedStructStatement*>(statement);
		if (ptr) {
			auto s = emit_struct_statement(ptr);
			return s;
		}
	}
    
	{
		auto ptr = dynamic_cast<TypedAssigmentStatement*>(statement);
		if (ptr) {
			auto s = emit_assigment_statement(ptr);
			return s;
		}
	}
    
	{
		auto ptr = dynamic_cast<TypedExpressionStatement*>(statement);
		if (ptr) {
			return emit_expression_statement(ptr);
		}
	}
    
	panic("Oh no");
}

std::string CBackend::emit_insert_statement(TypedInsertStatement* statement) {
	auto string_lit = dynamic_cast<StringLiteralExpression*>(statement->code);
	if (string_lit) {
		return string_lit->token.string; // 
	}
	panic("Insert only accepts string literals");
	throw;	
}

std::string CBackend::emit_return_statement(TypedReturnStatement* statement) {
	auto expression = emit_expression(statement->expression);
	if (expression.empty())
		return "ret\n";
    
	expression.append("pop_ret\n");
	return expression;
}

std::string CBackend::emit_break_statement(TypedBreakStatement* statement) {
	return "goto " + statement->identifier.string + ";\n";
}


std::string CBackend::emit_let_statement(TypedLetStatement* statement) {
	auto source = std::string();
	source.append(emit_expression(statement->type_expression) + " ");
	source.append(statement->identifier.string + " = ");
	auto emitted_expr = emit_expression(statement->expression);
	source.append(emitted_expr + ";\n");
	return source;
}

std::string CBackend::emit_scope_statement(TypedScopeStatement* statement) {
	auto fn_source = std::string("{\n");
	for (auto stmt : statement->statements) {
		fn_source.append(emit_statement(stmt));
	}
	fn_source.append("\n}\n");
	return fn_source;
}

std::string CBackend::emit_fn_statement(TypedFnStatement* statement) {
	auto fn_source = std::string();
	fn_source.append(emit_expression(statement->return_type) + " ");
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

std::string CBackend::emit_loop_statement(TypedLoopStatement* statement) {
	auto source = std::string();
	source.append("while(true)");
	source.append(emit_scope_statement(statement->body));
	source.append(statement->identifier.string + ": ;\n");
	return source;
}

std::string CBackend::emit_struct_statement(TypedStructStatement* statement) {
	auto source = std::string();
	source.append("struct " + statement->identifier.string + "{\n");
	for (auto& [identifier, type] : statement->members) {
		source.append(emit_expression(type) + " " + identifier.string + ";\n");
	}
	source.append("\n};\n\n");
	return source;
}

std::string CBackend::emit_assigment_statement(TypedAssigmentStatement* statement) {
	auto source = std::string();
	source.append(statement->identifier.string + " = ");
	source.append(emit_expression(statement->assigned_to));
	source.append(";\n");
	return source;
}

std::string CBackend::emit_expression_statement(TypedExpressionStatement* statement) {
	return emit_expression(statement->expression) + ";\n";
}

std::string CBackend::emit_expression(TypedExpression* expression) {
	{
		auto ptr = dynamic_cast<TypedStringLiteralExpression*>(expression);
		if (ptr) {
			return emit_string_literal_expression(ptr);
		}
	}
    
	{
		auto ptr = dynamic_cast<TypedIntLiteralExpression*>(expression);
		if (ptr) {
			return emit_int_literal_expression(ptr);
		}
	}
    
	{
		auto ptr = dynamic_cast<TypedCallExpression*>(expression);
		if (ptr) {
			return emit_call_expression(ptr);
		}
	}
    
	{
		auto ptr = dynamic_cast<TypedIdentifierExpression*>(expression);
		if (ptr) {
			return emit_identifier_expression(ptr);
		}
	}
    
	{
		auto ptr = dynamic_cast<TypedBinaryExpression*>(expression);
		if (ptr) {
			return emit_binary_expression(ptr);
		}
	}
    
	{
		auto ptr = dynamic_cast<TypedUnaryExpression*>(expression);
		if (ptr) {
			return emit_unary_expression(ptr);
		}
	}
    
	{
		auto ptr = dynamic_cast<TypedGetExpression*>(expression);
		if (ptr) {
			return emit_get_expression(ptr);
		}
	}
    
	{
		auto ptr = dynamic_cast<TypedNewExpression*>(expression);
		if (ptr) {
			return emit_new_expression(ptr);
		}
	}
    
	// given empty expression
	return "";
}

std::string CBackend::emit_binary_expression(TypedBinaryExpression* expression) {
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

std::string CBackend::emit_int_literal_expression(TypedIntLiteralExpression* expression) {
	return expression->token.string;
}

std::string CBackend::emit_string_literal_expression(TypedStringLiteralExpression* expression) {
	return "\"" + expression->token.string + "\"";
}

std::string CBackend::emit_call_expression(TypedCallExpression* expression) {
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

std::string CBackend::emit_unary_expression(TypedUnaryExpression* expression) {
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

std::string CBackend::emit_identifier_expression(TypedIdentifierExpression* expression) {
	return expression->identifier.string;
}

std::string CBackend::emit_get_expression(TypedGetExpression* expression) {
	return emit_expression(expression->expression) + "." + expression->member.string;
}

std::string CBackend::emit_new_expression(TypedNewExpression* expression) {
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