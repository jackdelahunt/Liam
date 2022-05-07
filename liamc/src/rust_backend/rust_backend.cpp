#include "rust_backend/rust_backend.h"

#include "statement.h"
#include "liam.h"

std::string RustBackend::emit(TypedFile& file) {
	auto source_generated = std::string(""
                                        "#![allow(unused_unsafe)]\n"
                                        "#![allow(non_camel_case_types)]\n"
                                        "#![allow(dead_code)]\n"
                                        "#![allow(unused_mut)]\n"
                                        "#![allow(unused_variables)]\n"
                                        "type void = ();\n"
                                        "type string = String;\n\n\n");
	for (auto stmt : file.statements) {
		source_generated.append(emit_statement(stmt));
	}

	return source_generated;
}

std::string RustBackend::emit_statement(TypedStatement* statement) {
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
    return nullptr;
}

std::string RustBackend::emit_insert_statement(TypedInsertStatement* statement) {
    return statement->code->token.string;
}

std::string RustBackend::emit_return_statement(TypedReturnStatement* statement) {
	auto expression = emit_expression(statement->expression);
	if (expression.empty())
		return "ret\n";

	expression.append("pop_ret\n");
	return expression;
}

std::string RustBackend::emit_break_statement(TypedBreakStatement* statement) {
	return "goto " + statement->identifier.string + ";\n";
}


std::string RustBackend::emit_let_statement(TypedLetStatement* statement) {
	auto source = std::string("let mut ");
	source.append(statement->identifier.string + ": ");
	source.append(emit_expression(statement->type_expression) + " = ");
	auto emitted_expr = emit_expression(statement->expression);
	source.append(emitted_expr + ";\n");
	return source;
}

std::string RustBackend::emit_scope_statement(TypedScopeStatement* statement) {
	auto fn_source = std::string("{ unsafe {\n");
	for (auto stmt : statement->statements) {
		fn_source.append(emit_statement(stmt));
	}
	fn_source.append("\n}}\n");
	return fn_source;
}

std::string RustBackend::emit_fn_statement(TypedFnStatement* statement) {
	auto fn_source = std::string();
	fn_source.append("fn ");
	fn_source.append(statement->identifier.string);
	fn_source.append("(");	
	
	int index = 0;
	for (auto& [identifier, type] : statement->params) {
		fn_source.append(identifier.string + ": " + emit_expression(type));
		index++;
		if (index < statement->params.size()) {
			fn_source.append(", ");
		}
	}
	fn_source.append(")");

	fn_source.append(" -> " + emit_expression(statement->return_type) + " ");

	fn_source.append(emit_scope_statement(statement->body));

	return fn_source + "\n";
}

std::string RustBackend::emit_loop_statement(TypedLoopStatement* statement) {
	auto source = std::string();
	source.append("while(true)");
	source.append(emit_scope_statement(statement->body));
	source.append(statement->identifier.string + ": ;\n");
	return source;
}

std::string RustBackend::emit_struct_statement(TypedStructStatement* statement) {
	auto source = std::string();
	source.append("#[derive(Clone)]\n");
	source.append("struct " + statement->identifier.string + "{\n");
	for (auto& [identifier, type] : statement->members) {
		source.append(identifier.string + ": " + emit_expression(type) + ",\n");
	}
	source.append("\n}\n\n");

    // struct impl block
    source.append("impl " + statement->identifier.string + " {\n");
    source.append(" fn new(");
    for (int i = 0; i < statement->members.size(); i++) {
        auto& [identifier, type] = statement->members.at(i);
        source.append(identifier.string + ": " + emit_expression(type));

        if(i + 1 < statement->members.size()) {
            source.append(", ");
        }
    }
    source.append(") -> Self {\n");
    source.append("     Self{");
    for (int i = 0; i < statement->members.size(); i++) {
        auto& [identifier, _] = statement->members.at(i);
        source.append(identifier.string);
        if(i + 1 < statement->members.size()) {
            source.append(", ");
        }
    }
    source.append("}\n");
    source.append(" }\n");
    source.append("}\n\n");

	return source;
}

std::string RustBackend::emit_assigment_statement(TypedAssigmentStatement* statement) {
	auto source = std::string();
	source.append(statement->identifier.string + " = ");
	source.append(emit_expression(statement->assigned_to));
	source.append(";\n");
	return source;
}

std::string RustBackend::emit_expression_statement(TypedExpressionStatement* statement) {
	return emit_expression(statement->expression) + ";\n";
}

std::string RustBackend::emit_expression(TypedExpression* expression) {
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

std::string RustBackend::emit_binary_expression(TypedBinaryExpression* expression) {
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

std::string RustBackend::emit_int_literal_expression(TypedIntLiteralExpression* expression) {
	return expression->token.string;
}

std::string RustBackend::emit_string_literal_expression(TypedStringLiteralExpression* expression) {
	return "String::from(\"" + expression->token.string + "\")";
}

std::string RustBackend::emit_call_expression(TypedCallExpression* expression) {
	auto source = std::string();
	source.append(dynamic_cast<TypedIdentifierExpression*>(expression->identifier)->identifier.string + "(");
	int index = 0;
	for (auto expr : expression->args) {

        if(expr->type_info->type != POINTER) {
		    source.append("(");
            source.append(emit_expression(expr));
            source.append(").clone()");
        } else {
            source.append(emit_expression(expr));
        }

		index++;
		if (index < expression->args.size()) {
			source.append(", ");
		}
	}
	source.append(")");

	return source;
}

std::string RustBackend::emit_unary_expression(TypedUnaryExpression* expression) {
	// int x = 10;
	// int* a = &x;
	// int b = *a;
	
	// let x: u64 = 10;
	// let a: u64^ = @x;
	// let b: u64 = *a;

	if (expression->op.type == TOKEN_HAT) {
		return "*const " + emit_expression(expression->expression);
	}
	else if (expression->op.type == TOKEN_AT) {
		return "&" + emit_expression(expression->expression);
	}
	else if (expression->op.type == TOKEN_STAR) {
		return emit_expression(expression->expression) + ".read()";
	}

	panic("Got a unrecognized operand");
    return nullptr;
}

std::string RustBackend::emit_identifier_expression(TypedIdentifierExpression* expression) {
	return expression->identifier.string;
}

std::string RustBackend::emit_get_expression(TypedGetExpression* expression) {
	return emit_expression(expression->expression) + "." + expression->member.string;
}

std::string RustBackend::emit_new_expression(TypedNewExpression* expression) {
	std::string source = expression->identifier.string + "::new(";
	int index = 0;
	for (auto expr : expression->expressions) {
		source.append(emit_expression(expr));
		index++;
		if (index < expression->expressions.size()) {
			source.append(", ");
		}
	}
	source.append(")");

	return source;
}