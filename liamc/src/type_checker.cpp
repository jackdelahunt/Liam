#include "type_checker.h"
#include "parser.h"
#include <cassert>  

TypeReference::TypeReference(Type type, size_t pointer_level) {
	this->type = type;
	this->pointer_level = pointer_level;
}

SymbolTable::SymbolTable() {
	this->types = { 
		{"void", TypeReference(VOID, 0)}, 
		{"u64", TypeReference(U64, 0)},
		{"string", TypeReference(STRING, 0)},
	};
}

TypedExpression::TypedExpression(Expression* expression, TypeReference* type_reference, size_t pointer_level) {
	this->expression = expression;					// evealuated expression
	this->type_reference = type_reference;			// that expressions type in the symbol table
	this->pointer_level_offset = pointer_level;		// current pointer level offset to type reference
}

TypeChecker::TypeChecker() {};

void TypeChecker::type_check(File* root) {
	auto symbol_table = SymbolTable();
	for (auto stmt : root->statements) {
		type_check_statement(stmt, &symbol_table);
	}
}

void TypeChecker::type_check_statement(Statement* statement, SymbolTable* symbol_table) {
	{
		auto ptr = dynamic_cast<LetStatement*>(statement);
		if (ptr) {
			type_check_let_statement(ptr, symbol_table);
		}
	}
}

void TypeChecker::type_check_let_statement(LetStatement* statement, SymbolTable* symbol_table) {
	auto let_typed_expression = type_check_expression(statement->type, symbol_table);
	auto typed_expression = type_check_expression(statement->expression, symbol_table);
	
	if (!matched_type(&let_typed_expression, &typed_expression)) {
		panic("Mismatched types");
	}

	symbol_table->types[statement->identifier.string] = TypeReference(let_typed_expression.type_reference->type, let_typed_expression.pointer_level_offset);
	return;
}

TypedExpression TypeChecker::type_check_expression(Expression* expression, SymbolTable* symbol_table) {
	{
		auto ptr = dynamic_cast<IdentifierExpression*>(expression);
		if (ptr) {
			return type_check_identifier_expression(ptr, symbol_table);
		}
	}

	{
		auto ptr = dynamic_cast<UnaryExpression*>(expression);
		if (ptr) {
			return type_check_unary_expression(ptr, symbol_table);
		}
	}

	{
		auto ptr = dynamic_cast<IntLiteralExpression*>(expression);
		if (ptr) {
			return type_check_int_literal_expression(ptr, symbol_table);
		}
	}

	{
		auto ptr = dynamic_cast<StringLiteralExpression*>(expression);
		if (ptr) {
			return type_check_string_literal_expression(ptr, symbol_table);
		}
	}

	{
		auto ptr = dynamic_cast<TypeLiteralExpression*>(expression);
		if (ptr) {
			return type_check_type_literal_expression(ptr, symbol_table);
		}
	}

	panic("cannot type check this expression");
}

TypedExpression TypeChecker::type_check_identifier_expression(IdentifierExpression* expression, SymbolTable* symbol_table) {
	if (!symbol_table->types.contains(expression->identifier.string)) {
		panic("unknown symbol");
	}

	auto t_ref = &symbol_table->types[expression->identifier.string];
	return TypedExpression(expression, t_ref, 0);
}


TypedExpression TypeChecker::type_check_unary_expression(UnaryExpression* expression, SymbolTable* symbol_table) {
	if (expression->op.type == TOKEN_HAT || expression->op.type == TOKEN_AT) {
		auto sub_expression_type = type_check_expression(expression->expression, symbol_table);
		return TypedExpression(expression, sub_expression_type.type_reference, 1);
	} else if (expression->op.type == TOKEN_STAR) {
		auto sub_expression_type = type_check_expression(expression->expression, symbol_table);
		return TypedExpression(expression, sub_expression_type.type_reference, -1);
	}

	panic("cannot type check this unary expression");
}

TypedExpression TypeChecker::type_check_type_literal_expression(TypeLiteralExpression* expression, SymbolTable* symbol_table) {
	assert(symbol_table->types.contains(expression->token.string));
	return TypedExpression(expression, &symbol_table->types[expression->token.string], 0);
}

TypedExpression TypeChecker::type_check_int_literal_expression(IntLiteralExpression* expression, SymbolTable* symbol_table) {
	return TypedExpression(expression, &symbol_table->types["u64"], 0);
}

TypedExpression TypeChecker::type_check_string_literal_expression(StringLiteralExpression* expression, SymbolTable* symbol_table) {
	return TypedExpression(expression, &symbol_table->types["string"], 0);
}

bool matched_type(TypedExpression* a, TypedExpression* b) {
	if (a->type_reference->type != b->type_reference->type) {
		panic("incompatible types");
		return false;
	}

	return a->type_reference->pointer_level + a->pointer_level_offset == b->type_reference->pointer_level + b->pointer_level_offset;
}