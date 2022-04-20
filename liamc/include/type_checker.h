#pragma once
#include "parser.h"
#include <map>

struct Statement;
struct LetStatement;
struct ScopeStatement;
struct StructStatement;
struct FnStatement;
struct LoopStatement;
struct InsertStatement;
struct ReturnStatement;
struct BreakStatement;
struct ExpressionStatement;
struct AssigmentStatement;
struct IntLiteralExpression;
struct TypeLiteralExpression;
struct StringLiteralExpression;
struct Expression;
struct File;

enum Type {
	VOID,
	U64,
	STRING
};

struct TypeReference {
	Type type;
	size_t pointer_level;

	TypeReference() = default;
	TypeReference(Type type, size_t pointer_level);
};

struct SymbolTable {
	std::map<std::string, TypeReference> types;

	SymbolTable();
};

struct TypedExpression {
	Expression* expression;
	TypeReference* type_reference;
	size_t pointer_level_offset;

	TypedExpression(Expression* expression, TypeReference* type_reference, size_t pointer_level);
};


struct TypeChecker {


	TypeChecker();
	void type_check(File* root);

	void type_check_statement(Statement* statement, SymbolTable* symbol_table);
	void type_check_let_statement(LetStatement* statement, SymbolTable* symbol_table);

	TypedExpression type_check_expression(Expression* expression, SymbolTable* symbol_table);
	TypedExpression type_check_identifier_expression(IdentifierExpression* expression, SymbolTable* symbol_table);
	TypedExpression type_check_unary_expression(UnaryExpression* expression, SymbolTable* symbol_table);
	TypedExpression type_check_int_literal_expression(IntLiteralExpression* expression, SymbolTable* symbol_table);
	TypedExpression type_check_string_literal_expression(StringLiteralExpression* expression, SymbolTable* symbol_table);
	TypedExpression type_check_type_literal_expression(TypeLiteralExpression* expression, SymbolTable* symbol_table);

};

bool matched_type(TypedExpression* a, TypedExpression* b);