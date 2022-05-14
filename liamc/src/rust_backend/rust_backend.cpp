#include "rust_backend/rust_backend.h"

#include "statement.h"
#include "liam.h"

std::string RustBackend::
emit(File* file) {
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
                                        "#![allow(unused_parens)]"
                                        "type void = ();");

	for (auto stmt : file->statements) {
		source_generated.append(emit_statement(stmt));
	}

	return source_generated;
}

std::string RustBackend::
emit_statement(Statement* statement) {
    switch (statement->statement_type) {
        case StatementType::STATEMENT_INSERT:
            return emit_insert_statement(dynamic_cast<InsertStatement*>(statement));
            break;
        case StatementType::STATEMENT_RETURN:
            return emit_return_statement(dynamic_cast<ReturnStatement*>(statement));
            break;
        case StatementType::STATEMENT_BREAK:
            return emit_break_statement(dynamic_cast<BreakStatement*>(statement));
            break;
        case StatementType::STATEMENT_LET:
            return emit_let_statement(dynamic_cast<LetStatement*>(statement));
            break;
        case StatementType::STATEMENT_FN:
            return emit_fn_statement(dynamic_cast<FnStatement*>(statement));
            break;
        case StatementType::STATEMENT_SCOPE:
            return emit_scope_statement(dynamic_cast<ScopeStatement*>(statement));
            break;
        case StatementType::STATEMENT_LOOP:
            return emit_loop_statement(dynamic_cast<LoopStatement*>(statement));
            break;
        case StatementType::STATEMENT_STRUCT:
            return emit_struct_statement(dynamic_cast<StructStatement*>(statement));
            break;
        case StatementType::STATEMENT_ASSIGNMENT:
            return emit_assigment_statement(dynamic_cast<AssigmentStatement*>(statement));
            break;
        case StatementType::STATEMENT_EXPRESSION:
            return emit_expression_statement(dynamic_cast<ExpressionStatement*>(statement));
            break;
        case StatementType::STATEMENT_FOR:
            return emit_for_statement(dynamic_cast<ForStatement*>(statement));
            break;
        case StatementType::STATEMENT_IF:
            return emit_if_statement(dynamic_cast<IfStatement*>(statement));
            break;
    }


	panic("Statement not implemented in rust back end :[");
    return "";
}

std::string RustBackend::
emit_insert_statement(InsertStatement* statement) {
    auto string_lit = dynamic_cast<StringLiteralExpression*>(statement->byte_code);
    return string_lit->token.string;
}

std::string RustBackend::
emit_return_statement(ReturnStatement* statement) {
	return "return " + emit_expression(statement->expression) + ";";
}

std::string RustBackend::
emit_break_statement(BreakStatement* statement) {
	return "goto " + statement->identifier.string + ";";
}


std::string RustBackend::
emit_let_statement(LetStatement* statement) {
	auto source = std::string("let mut ");
	source.append(statement->identifier.string);
    if(statement->type) {
	    source.append(": " + emit_type_expression(statement->type) + " = ");
    } else {
        source.append(" = ");
    }
	auto emitted_expr = emit_expression(statement->rhs);
	source.append(emitted_expr + ";");
	return source;
}

std::string RustBackend::
emit_scope_statement(ScopeStatement* statement) {
	auto fn_source = std::string("{ unsafe {");
	for (auto stmt : statement->statements) {
		fn_source.append(emit_statement(stmt));
	}
	fn_source.append("}}");
	return fn_source;
}

std::string RustBackend::
emit_fn_statement(FnStatement* statement) {
	auto fn_source = std::string();
	fn_source.append("fn ");
	fn_source.append(statement->identifier.string);
	fn_source.append("(");	
	
	s32 index = 0;
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
emit_loop_statement(LoopStatement* statement) {
	auto source = std::string();
	source.append("while(true)");
	source.append(emit_scope_statement(statement->body));
	source.append(statement->identifier.string + ": ;");
	return source;
}

std::string RustBackend::
emit_struct_statement(StructStatement* statement) {
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
    for (s32 i = 0; i < statement->members.size(); i++) {
        auto& [identifier, type] = statement->members.at(i);
        source.append(identifier.string + ": " + emit_type_expression(type));

        if(i + 1 < statement->members.size()) {
            source.append(", ");
        }
    }
    source.append(") -> Self {");
    source.append("Self{");
    for (s32 i = 0; i < statement->members.size(); i++) {
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
emit_assigment_statement(AssigmentStatement* statement) {
	auto source = std::string();
	source.append(statement->identifier.string + " = ");
	source.append(emit_expression(statement->assigned_to->expression));
	source.append(";");
	return source;
}

std::string RustBackend::
emit_expression_statement(ExpressionStatement* statement) {
	return emit_expression(statement->expression) + ";";
}

std::string RustBackend::
emit_for_statement(ForStatement* statement) {
    std::string source ="";
    source.append("for (___i___generated, " + statement->value_identifier.string + ") in (" + emit_expression(statement->array_expression));
    source.append(").iter_mut().enumerate() { let " + statement->index_identifier.string + " = ___i___generated as u64;");
    source.append(emit_scope_statement(statement->body));
    source.append("}");
    return source;
}

std::string RustBackend::
emit_if_statement(IfStatement* statement) {
    auto source = std::string("if (");
    source.append(emit_expression(statement->expression) + ")");
    source.append(emit_scope_statement(statement->body));
    return source;
}

std::string RustBackend::
emit_expression(Expression* expression) {
	{
        switch (expression->type) {
            case ExpressionType::EXPRESSION_STRING_LITERAL:
                return emit_string_literal_expression(dynamic_cast<StringLiteralExpression*>(expression));
                break;
            case ExpressionType::EXPRESSION_INT_LITERAL:
                return emit_int_literal_expression(dynamic_cast<IntLiteralExpression*>(expression));
                break;
            case ExpressionType::EXPRESSION_BOOL_LITERAL:
                return emit_bool_literal_expression(dynamic_cast<BoolLiteralExpression*>(expression));
                break;
            case ExpressionType::EXPRESSION_CALL:
                return emit_call_expression(dynamic_cast<CallExpression*>(expression));
                break;
            case ExpressionType::EXPRESSION_IDENTIFIER:
                return emit_identifier_expression(dynamic_cast<IdentifierExpression*>(expression));
                break;
            case ExpressionType::EXPRESSION_BINARY:
                return emit_binary_expression(dynamic_cast<BinaryExpression*>(expression));
                break;
            case ExpressionType::EXPRESSION_UNARY:
                return emit_unary_expression(dynamic_cast<UnaryExpression*>(expression));
                break;
            case ExpressionType::EXPRESSION_GET:
                return emit_get_expression(dynamic_cast<GetExpression*>(expression));
                break;
            case ExpressionType::EXPRESSION_NEW:
                return emit_new_expression(dynamic_cast<NewExpression*>(expression));
                break;
            case ExpressionType::EXPRESSION_ARRAY:
                return emit_array_expression(dynamic_cast<ArrayExpression*>(expression));
                break;
            case ExpressionType::EXPRESSION_ARRAY_SUBSCRIPT:
                return emit_array_subscript_expression(dynamic_cast<ArraySubscriptExpression*>(expression));
                break;
            case ExpressionType::EXPRESSION_GROUP:
                return emit_group_expression(dynamic_cast<GroupExpression*>(expression));
                break;
            default:
                return "";
        }
	}
}

std::string RustBackend::
emit_cloneable_expression(Expression* expression) {
    // no need to clone pointer, literals or new structs
    if(expression->type_info->type == TypeInfoType::POINTER
       || expression->type == ExpressionType::EXPRESSION_STRING_LITERAL
       || expression->type == ExpressionType::EXPRESSION_INT_LITERAL
       || expression->type == ExpressionType::EXPRESSION_BOOL_LITERAL
       || expression->type == ExpressionType::EXPRESSION_NEW
       || expression->type == ExpressionType::EXPRESSION_ARRAY
       || expression->type_info->type == TypeInfoType::INT
       || expression->type_info->type == TypeInfoType::BOOL
       || expression->type_info->type == TypeInfoType::CHAR
    ) {
        return emit_expression(expression);
    }

    return "(" + emit_expression(expression) + ").clone()";
}

std::string RustBackend::
emit_binary_expression(BinaryExpression* expression) {
	auto source = std::string();
	source.append(emit_expression(expression->left));
	switch (expression->op.type)
	{
	case TOKEN_PLUS:
		source.append("+");
		break;
    case TOKEN_STAR:
        source.append("*");
        break;
    case TOKEN_OR:
        source.append("||");
        break;
    case TOKEN_AND:
        source.append("&&");
        break;
    case TOKEN_EQUAL:
        source.append("==");
        break;
    case TOKEN_NOT_EQUAL:
        source.append("!=");
        break;
	default:
		panic("Cannot use this operand");
		break;
	}
	source.append(emit_expression(expression->right));

	return source;
}

std::string RustBackend::
emit_int_literal_expression(IntLiteralExpression* expression) {
	return expression->token.string;
}

std::string RustBackend::
emit_string_literal_expression(StringLiteralExpression* expression) {
	return "String::from(\"" + expression->token.string + "\").into_bytes()";
}

std::string RustBackend::
emit_bool_literal_expression(BoolLiteralExpression* expression) {
    return expression->value.string;
}

std::string RustBackend::
emit_call_expression(CallExpression* expression) {
	auto source = std::string();
    auto identifier = dynamic_cast<IdentifierExpression*>(expression->identifier);
	source.append(identifier->identifier.string + "(");
	s32 index = 0;
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
emit_unary_expression(UnaryExpression* expression) {
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
emit_identifier_expression(IdentifierExpression* expression) {
	return expression->identifier.string;
}

std::string RustBackend::
emit_get_expression(GetExpression* expression) {
	return emit_expression(expression->lhs) + "." + expression->member.string;
}

std::string RustBackend::
emit_new_expression(NewExpression* expression) {
	std::string source = expression->identifier.string + "::new(";
	s32 index = 0;
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
emit_array_expression(ArrayExpression* expression) {
    std::string source = "vec![";
    s32 index = 0;
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
emit_array_subscript_expression(ArraySubscriptExpression* expression) {
    std::string source = "(" + emit_expression(expression->array);
    source.append("[");
    source.append(emit_cloneable_expression(expression->subscript));
    source.append("]).clone()");
    return source;
}

std::string RustBackend::
emit_group_expression(GroupExpression* expression) {
    return "(" + emit_expression(expression->expression) + ")";
}

std::string RustBackend::
emit_type_expression(TypeExpression *type_expression) {
    switch (type_expression->type) {
        case TypeExpressionType::TYPE_IDENTIFIER:
            return emit_identifier_type_expression(dynamic_cast<IdentifierTypeExpression*>(type_expression)); break;
        case TypeExpressionType::TYPE_POINTER:
            return emit_pointer_type_expression(dynamic_cast<PointerTypeExpression*>(type_expression)); break;
        case TypeExpressionType::TYPE_ARRAY:
            return emit_array_type_expression(dynamic_cast<ArrayTypeExpression*>(type_expression)); break;
        default:
            panic("Rust back end does not support this return_type expression");
    }
}

std::string RustBackend::
emit_identifier_type_expression(IdentifierTypeExpression* type_expression) {
    // the identifier char already exists in rust, but we actually want u8
    if(type_expression->type_info->type == TypeInfoType::CHAR) {
        return "u8";
    }
    return type_expression->identifier.string;
}

std::string RustBackend::
emit_pointer_type_expression(PointerTypeExpression* type_expression) {
    return "*mut " + emit_type_expression(type_expression->pointer_of);
}
std::string RustBackend::
emit_array_type_expression(ArrayTypeExpression* type_expression) {
    return "Vec<" + emit_type_expression(type_expression->array_of) + ">";
}