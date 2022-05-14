#include "rust_backend/rust_backend.h"

#include "statement.h"
#include "liam.h"

std::string RustBackend::
emit(TypedFile* file) {
	auto source_generated = std::string(""
                                        "#![allow(unused_unsafe)]"
                                        "#![allow(non_camel_case_types)]"
                                        "#![allow(dead_code)]"
                                        "#![allow(unused_mut)]"
                                        "#![allow(unused_variables)]"
                                        "#![allow(unused_assignments)]"
                                        "#![allow(non_snake_case)]"
                                        "#![allow(unused_braces)]"
                                        "#![allow(unreachable_code)]"
                                        "#![allow(unused_imports)]"
                                        "type void = ();"
                                        "type string = String;");

	for (auto stmt : file->statements) {
		source_generated.append(emit_statement(stmt));
	}

	return source_generated;
}

std::string RustBackend::
emit_statement(TypeCheckedStatement* statement) {
	{
		auto ptr = dynamic_cast<TypeCheckedInsertStatement*>(statement);
		if (ptr) {
			return emit_insert_statement(ptr);
		}
	}

	{
		auto ptr = dynamic_cast<TypeCheckedReturnStatement*>(statement);
		if (ptr) {
			return emit_return_statement(ptr);
		}
	}

	{
		auto ptr = dynamic_cast<TypeCheckedBreakStatement*>(statement);
		if (ptr) {
			return emit_break_statement(ptr);
		}
	}

	{
		auto ptr = dynamic_cast<TypeCheckedLetStatement*>(statement);
		if (ptr) {
			return emit_let_statement(ptr);
		}
	}

	{
		auto ptr = dynamic_cast<TypeCheckedFnStatement*>(statement);
		if (ptr) {
			auto s = emit_fn_statement(ptr);
			return s;
		}
	}

	{
		auto ptr = dynamic_cast<TypeCheckedLoopStatement*>(statement);
		if (ptr) {
			auto s = emit_loop_statement(ptr);
			return s;
		}
	}

	{
		auto ptr = dynamic_cast<TypeCheckedStructStatement*>(statement);
		if (ptr) {
			auto s = emit_struct_statement(ptr);
			return s;
		}
	}

	{
		auto ptr = dynamic_cast<TypeCheckedAssigmentStatement*>(statement);
		if (ptr) {
			auto s = emit_assigment_statement(ptr);
			return s;
		}
	}

	{
		auto ptr = dynamic_cast<TypeCheckedExpressionStatement*>(statement);
		if (ptr) {
			return emit_expression_statement(ptr);
		}
	}

    {
        auto ptr = dynamic_cast<TypeCheckedForStatement*>(statement);
        if (ptr) {
            return emit_for_statement(ptr);
        }
    }

    {
        auto ptr = dynamic_cast<TypeCheckedIfStatement*>(statement);
        if (ptr) {
            return emit_if_statement(ptr);
        }
    }

	panic("Statement not implemented in rust back end :[");
    return nullptr;
}

std::string RustBackend::
emit_insert_statement(TypeCheckedInsertStatement* statement) {
    return statement->code->token.string + "";
}

std::string RustBackend::
emit_return_statement(TypeCheckedReturnStatement* statement) {
	return "return " + emit_expression(statement->expression) + ";";
}

std::string RustBackend::
emit_break_statement(TypeCheckedBreakStatement* statement) {
	return "goto " + statement->identifier.string + ";";
}


std::string RustBackend::
emit_let_statement(TypeCheckedLetStatement* statement) {
	auto source = std::string("let mut ");
	source.append(statement->identifier.string);
    if(statement->type_expression) {
	    source.append(": " + emit_type_expression(statement->type_expression) + " = ");
    } else {
        source.append(" = ");
    }
	auto emitted_expr = emit_expression(statement->expression);
	source.append(emitted_expr + ";");
	return source;
}

std::string RustBackend::
emit_scope_statement(TypeCheckedScopeStatement* statement) {
	auto fn_source = std::string("{ unsafe {");
	for (auto stmt : statement->statements) {
		fn_source.append(emit_statement(stmt));
	}
	fn_source.append("}}");
	return fn_source;
}

std::string RustBackend::
emit_fn_statement(TypeCheckedFnStatement* statement) {
	auto fn_source = std::string();
	fn_source.append("fn ");
	fn_source.append(statement->identifier.string);
	fn_source.append("(");	
	
	int index = 0;
	for (auto& [identifier, type] : statement->params) {
		fn_source.append(identifier.string + ": " + emit_type_expression(type));
		index++;
		if (index < statement->params.size()) {
			fn_source.append(", ");
		}
	}
	fn_source.append(")");

	fn_source.append(" -> " + emit_type_expression(statement->return_type) + " ");

	fn_source.append(emit_scope_statement(statement->body));

	return fn_source + "";
}

std::string RustBackend::
emit_loop_statement(TypeCheckedLoopStatement* statement) {
	auto source = std::string();
	source.append("while(true)");
	source.append(emit_scope_statement(statement->body));
	source.append(statement->identifier.string + ": ;");
	return source;
}

std::string RustBackend::
emit_struct_statement(TypeCheckedStructStatement* statement) {
	auto source = std::string();
	source.append("#[derive(Clone)]");
	source.append("struct " + statement->identifier.string + "{");
	for (auto& [identifier, type] : statement->members) {
		source.append(identifier.string + ": " + emit_type_expression(type) + ",");
	}
	source.append("}");

    // struct impl block
    source.append("impl " + statement->identifier.string + " {");
    source.append("fn new(");
    for (int i = 0; i < statement->members.size(); i++) {
        auto& [identifier, type] = statement->members.at(i);
        source.append(identifier.string + ": " + emit_type_expression(type));

        if(i + 1 < statement->members.size()) {
            source.append(", ");
        }
    }
    source.append(") -> Self {");
    source.append("Self{");
    for (int i = 0; i < statement->members.size(); i++) {
        auto& [identifier, _] = statement->members.at(i);
        source.append(identifier.string);
        if(i + 1 < statement->members.size()) {
            source.append(", ");
        }
    }
    source.append("}");
    source.append("}");
    source.append("}");

	return source;
}

std::string RustBackend::
emit_assigment_statement(TypeCheckedAssigmentStatement* statement) {
	auto source = std::string();
	source.append(statement->identifier.string + " = ");
	source.append(emit_expression(statement->assigned_to));
	source.append(";");
	return source;
}

std::string RustBackend::
emit_expression_statement(TypeCheckedExpressionStatement* statement) {
	return emit_expression(statement->expression) + ";";
}

std::string RustBackend::
emit_for_statement(TypeCheckedForStatement* statement) {
    std::string source ="";
    source.append("for (___i___generated, " + statement->value_identifier.string + ") in " + emit_expression(statement->array_expression));
    source.append(".iter_mut().enumerate() { let " + statement->index_identifier.string + " = ___i___generated as u64;");
    source.append(emit_scope_statement(statement->body));
    source.append("}");
    return source;
}

std::string RustBackend::
emit_if_statement(TypeCheckedIfStatement* statement) {
    auto source = std::string("if ");
    source.append(emit_expression(statement->expression));
    source.append(emit_scope_statement(statement->body));
    return source;
}

std::string RustBackend::
emit_expression(TypeCheckedExpression* expression) {
	{
        switch (expression->type) {
            case ExpressionType::EXPRESSION_STRING_LITERAL:
                return emit_string_literal_expression(dynamic_cast<TypeCheckedStringLiteralExpression*>(expression));
                break;
            case ExpressionType::EXPRESSION_INT_LITERAL:
                return emit_int_literal_expression(dynamic_cast<TypeCheckedIntLiteralExpression*>(expression));
                break;
            case ExpressionType::EXPRESSION_BOOL_LITERAL:
                return emit_bool_literal_expression(dynamic_cast<TypeCheckedBoolLiteralExpression*>(expression));
                break;
            case ExpressionType::EXPRESSION_CALL:
                return emit_call_expression(dynamic_cast<TypeCheckedCallExpression*>(expression));
                break;
            case ExpressionType::EXPRESSION_IDENTIFIER:
                return emit_identifier_expression(dynamic_cast<TypeCheckedIdentifierExpression*>(expression));
                break;
            case ExpressionType::EXPRESSION_BINARY:
                return emit_binary_expression(dynamic_cast<TypeCheckedBinaryExpression*>(expression));
                break;
            case ExpressionType::EXPRESSION_UNARY:
                return emit_unary_expression(dynamic_cast<TypeCheckedUnaryExpression*>(expression));
                break;
            case ExpressionType::EXPRESSION_GET:
                return emit_get_expression(dynamic_cast<TypeCheckedGetExpression*>(expression));
                break;
            case ExpressionType::EXPRESSION_NEW:
                return emit_new_expression(dynamic_cast<TypeCheckedNewExpression*>(expression));
                break;
            case ExpressionType::EXPRESSION_ARRAY:
                return emit_array_expression(dynamic_cast<TypeCheckedArrayExpression*>(expression));
                break;
            case ExpressionType::EXPRESSION_ARRAY_SUBSCRIPT:
                return emit_array_subscript_expression(dynamic_cast<TypeCheckedArraySubscriptExpression*>(expression));
                break;
            default:
                return "";
        }
	}
}

std::string RustBackend::
emit_cloneable_expression(TypeCheckedExpression* expression) {
    // no need to clone pointer, literals or new structs
    if(expression->type_info->type == TypeInfoType::POINTER
       || expression->type == ExpressionType::EXPRESSION_STRING_LITERAL
       || expression->type == ExpressionType::EXPRESSION_INT_LITERAL
       || expression->type == ExpressionType::EXPRESSION_BOOL_LITERAL
       || expression->type == ExpressionType::EXPRESSION_NEW
       || expression->type == ExpressionType::EXPRESSION_ARRAY
       || expression->type_info->type == TypeInfoType::INT
       || expression->type_info->type == TypeInfoType::BOOL
    ) {
        return emit_expression(expression);
    }

    return "(" + emit_expression(expression) + ").clone()";
}

std::string RustBackend::
emit_binary_expression(TypeCheckedBinaryExpression* expression) {
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

std::string RustBackend::
emit_int_literal_expression(TypeCheckedIntLiteralExpression* expression) {
	return expression->token.string;
}

std::string RustBackend::
emit_string_literal_expression(TypeCheckedStringLiteralExpression* expression) {
	return "String::from(\"" + expression->token.string + "\")";
}

std::string RustBackend::
emit_bool_literal_expression(TypeCheckedBoolLiteralExpression* expression) {
    return expression->value.string;
}

std::string RustBackend::
emit_call_expression(TypeCheckedCallExpression* expression) {
	auto source = std::string();
    auto identifier = dynamic_cast<TypeCheckedIdentifierExpression*>(expression->identifier);
	source.append(identifier->identifier.string + "(");
	int index = 0;
	for (auto expr : expression->args) {
        source.append(emit_cloneable_expression(expr));
		index++;
		if (index < expression->args.size()) {
			source.append(", ");
		}
	}
	source.append(")");

	return source;
}

std::string RustBackend::
emit_unary_expression(TypeCheckedUnaryExpression* expression) {
	// int x = 10;
	// int* a = &x;
	// int b = *a;
	
	// let x: u64 = 10;
	// let a: u64^ = @x;
	// let b: u64 = *a;

	if (expression->op.type == TOKEN_AT) {
		return "(&mut " + emit_expression(expression->expression) + ") as *mut _";
	}
	else if (expression->op.type == TOKEN_STAR) {
		return "*" + emit_expression(expression->expression);
	}

	panic("Got a unrecognized operand");
    return nullptr;
}

std::string RustBackend::
emit_identifier_expression(TypeCheckedIdentifierExpression* expression) {
	return expression->identifier.string;
}

std::string RustBackend::
emit_get_expression(TypeCheckedGetExpression* expression) {
	return emit_expression(expression->expression) + "." + expression->member.string;
}

std::string RustBackend::
emit_new_expression(TypeCheckedNewExpression* expression) {
	std::string source = expression->identifier.string + "::new(";
	int index = 0;
	for (auto expr : expression->expressions) {
		source.append(emit_cloneable_expression(expr));
		index++;
		if (index < expression->expressions.size()) {
			source.append(", ");
		}
	}
	source.append(")");

	return source;
}

std::string RustBackend::
emit_array_expression(TypeCheckedArrayExpression* expression) {
    std::string source = "vec![";
    int index = 0;
    for (auto expr : expression->expressions) {
        source.append(emit_cloneable_expression(expr));
        index++;
        if (index < expression->expressions.size()) {
            source.append(", ");
        }
    }
    source.append("]");

    return source;
}

std::string RustBackend::
emit_array_subscript_expression(TypeCheckedArraySubscriptExpression* expression) {
    std::string source = "(" + emit_expression(expression->array);
    source.append("[");
    source.append(emit_cloneable_expression(expression->subscript));
    source.append("]).clone()");
    return source;
}

std::string RustBackend::
emit_type_expression(TypeCheckedTypeExpression *type_expression) {
    switch (type_expression->type) {
        case TypeExpressionType::TYPE_IDENTIFIER:
            return emit_identifier_type_expression(dynamic_cast<TypeCheckedIdentifierTypeExpression*>(type_expression)); break;
        case TypeExpressionType::TYPE_POINTER:
            return emit_pointer_type_expression(dynamic_cast<TypeCheckedPointerTypeExpression*>(type_expression)); break;
        case TypeExpressionType::TYPE_ARRAY:
            return emit_array_type_expression(dynamic_cast<TypeCheckedArrayTypeExpression*>(type_expression)); break;
        default:
            panic("Rust back end does not support this type expression");
    }
}

std::string RustBackend::
emit_identifier_type_expression(TypeCheckedIdentifierTypeExpression* type_expression) {
    return type_expression->identifier.string;
}

std::string RustBackend::
emit_pointer_type_expression(TypeCheckedPointerTypeExpression* type_expression) {
    return "*mut " + emit_type_expression(type_expression->pointer_of);
}
std::string RustBackend::
emit_array_type_expression(TypeCheckedArrayTypeExpression* type_expression) {
    return "Vec<" + emit_type_expression(type_expression->array_of) + ">";
}