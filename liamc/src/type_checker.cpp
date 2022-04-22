#include "type_checker.h"
#include "parser.h"
#include <cassert>  

SymbolTable::SymbolTable() {
	this->builtin_type_table = std::map<std::string, TypeInfo>();
	this->type_table = std::map<std::string, TypeInfo>();
	this->identifier_table = std::map<std::string, TypeInfo>();

	builtin_type_table["void"] = VoidTypeInfo{VOID};
	builtin_type_table["string"] = StringTypeInfo{STRING};
	builtin_type_table["u64"] = IntTypeInfo{INT, false, 64};
}

void SymbolTable::add_type(Token type, TypeInfo type_info) {
	if (type_table.contains(type.string)) {
		panic("Duplcate creation of type: " + type.string + " at (" + std::to_string(type.line) + "," + std::to_string(type.character) + ")");
	}

	type_table[type.string] = type_info;
}

void SymbolTable::add_identifier(Token identifier, TypeInfo type_info) {
	if (identifier_table.contains(identifier.string)) {
		panic("Duplcate creation of identifier: " + identifier.string + " at (" + std::to_string(identifier.line) + "," + std::to_string(identifier.character) + ")");
	}

	identifier_table[identifier.string] = type_info;
}

TypedLetStatement::TypedLetStatement(Token identifier, TypedExpression* type_expression, TypedExpression* expression) {
	this->identifier = identifier;
	this->type_expression = type_expression;
	this->expression = expression;
}

TypedScopeStatement::TypedScopeStatement(std::vector<TypedStatement*> statements) {
	this->statements = statements;
}

TypedStructStatement::TypedStructStatement(Token identifier, Typed_CSV members) {
	this->identifier = identifier;
	this->members= members;
}

TypedFnStatement::TypedFnStatement(Token identifier, Typed_CSV params, TypedExpression* return_type, TypedScopeStatement* body) {
	this->identifier = identifier;
	this->params = params;
	this->return_type = return_type;
	this->body = body;
}

TypedLoopStatement::TypedLoopStatement(Token identifier, TypedScopeStatement* body) {
	this->identifier = identifier;
	this->body = body;
}

TypedInsertStatement::TypedInsertStatement(TypedExpression* code) {
	this->code = code;
}

TypedReturnStatement::TypedReturnStatement(TypedExpression* epxression) {
	this->expression = expression;
}

TypedBreakStatement::TypedBreakStatement(Token identifier) {
	this->identifier = identifier;
}

TypedAssigmentStatement::TypedAssigmentStatement(Token identifier, TypedExpression* assigned_to) {
	this->identifier = identifier;
	this->assigned_to = assigned_to;
}

TypedBinaryExpression::TypedBinaryExpression(TypedExpression* left, Token op, TypedExpression* right) {
	this->left = left;
	this->op = op;
	this->right = right;
}

TypedUnaryExpression::TypedUnaryExpression(TypedExpression* expression, Token op) {
	this->expression = expression;
	this->op = op;
}

TypedIntLiteralExpression::TypedIntLiteralExpression(Token token) {
	this->token = token;
	this->type_info = IntTypeInfo{INT, true, 64};
}

TypedCallExpression::TypedCallExpression(TypedExpression* identifier, std::vector<TypedExpression*> args) {
	this->identifier = identifier;
	this->args = args;
}

TypedIdentifierExpression::TypedIdentifierExpression(Token identifier, TypeInfo type_info) {
	this->identifier;
	this->type_info = type_info;
}

TypedStringLiteralExpression::TypedStringLiteralExpression(Token token) {
	this->token = token;
	this->type_info = StringTypeInfo{ STRING };
}


TypedGetExpression::TypedGetExpression(TypedExpression* expression, Token member) {
	this->expression = expression;
	this->member = member;
}

TypedNewExpression::TypedNewExpression(Token identifier, std::vector<TypedExpression*> expressions) {
	this->identifier = identifier;
	this->expressions = expressions;
}


TypeChecker::TypeChecker() {
	symbol_table = SymbolTable();
	root = TypedFile();
}

void TypeChecker::type_file(File* file) {
	for (auto stmt : file->statements) {
		root.statements.push_back(type_statement(stmt, &symbol_table));
	}
}

TypedStatement* TypeChecker::type_statement(Statement* statement, SymbolTable* symbol_table) {

	{
		auto ptr = dynamic_cast<InsertStatement*>(statement);
		if (ptr) {
			return type_insert_statement(ptr, symbol_table);
		}
	}

	{
		auto ptr = dynamic_cast<ReturnStatement*>(statement);
		if (ptr) {
			return type_return_statement(ptr, symbol_table);
		}
	}

	{
		auto ptr = dynamic_cast<BreakStatement*>(statement);
		if (ptr) {
			return type_break_statement(ptr, symbol_table);
		}
	}

	{
		auto ptr = dynamic_cast<LetStatement*>(statement);
		if (ptr) {
			return type_let_statement(ptr, symbol_table);
		}
	}

	{
		auto ptr = dynamic_cast<FnStatement*>(statement);
		if (ptr) {
			return type_fn_statement(ptr, symbol_table);
		}
	}

	{
		auto ptr = dynamic_cast<LoopStatement*>(statement);
		if (ptr) {
			auto s = type_loop_statement(ptr, symbol_table);
			return s;
		}
	}

	{
		auto ptr = dynamic_cast<StructStatement*>(statement);
		if (ptr) {
			auto s = type_struct_statement(ptr, symbol_table);
			return s;
		}
	}

	{
		auto ptr = dynamic_cast<AssigmentStatement*>(statement);
		if (ptr) {
			auto s = type_assigment_statement(ptr, symbol_table);
			return s;
		}
	}

	{
		auto ptr = dynamic_cast<ExpressionStatement*>(statement);
		if (ptr) {
			return type_expression_statement(ptr, symbol_table);
		}
	}

	panic("Not implemented");
	return nullptr;
}
TypedInsertStatement* TypeChecker::type_insert_statement(InsertStatement* statement, SymbolTable* symbol_table) {
	panic("Not implemented");
	return nullptr;
}
TypedReturnStatement* TypeChecker::type_return_statement(ReturnStatement* statement, SymbolTable* symbol_table) {
	panic("Not implemented");
	return nullptr;
}
TypedBreakStatement* TypeChecker::type_break_statement(BreakStatement* statement, SymbolTable* symbol_table) {
	panic("Not implemented");
	return nullptr;
}
TypedLetStatement* TypeChecker::type_let_statement(LetStatement* statement, SymbolTable* symbol_table) {
	auto typed_type_expression = type_expression(statement->type, symbol_table);
	auto typed_expression = type_expression(statement->expression, symbol_table);

	if (!type_match(&typed_type_expression->type_info, &typed_expression->type_info)) {
		panic("Mis matched types in let statement");
	}

	symbol_table->add_identifier(statement->identifier, typed_type_expression->type_info);

	return new TypedLetStatement(
		statement->identifier, 
		typed_type_expression, 
		typed_expression
	);
}
TypedScopeStatement* TypeChecker::type_scope_statement(ScopeStatement* statement, SymbolTable* symbol_table, bool copy_symbol_table) {
	auto statements = std::vector<TypedStatement*>();

	// this is kind of a mess... oh jeez
	SymbolTable* scopes_symbol_table = symbol_table;
	SymbolTable possible_copy;
	if (copy_symbol_table) {
		possible_copy = *symbol_table;
		scopes_symbol_table = &possible_copy;
	}

	if (copy_symbol_table) {
	}

	for (auto stmt : statement->body) {
		statements.push_back(type_statement(stmt, scopes_symbol_table));
	}

	return new TypedScopeStatement(statements);
}
TypedFnStatement* TypeChecker::type_fn_statement(FnStatement* statement, SymbolTable* symbol_table) {

	auto param_type_infos = std::vector<TypeInfo>();
	Typed_CSV t_csv = Typed_CSV();
	for (auto& [identifier, expr] : statement->params) {
		auto p = type_expression(expr, symbol_table);
		param_type_infos.push_back(p->type_info);
		t_csv.push_back({identifier, p});
	}

	auto return_expresion = type_expression(statement->type, symbol_table);

	symbol_table->add_identifier(statement->identifier, FnTypeInfo{FN, return_expresion->type_info, param_type_infos});

	SymbolTable copied_symbol_table = *symbol_table;
	for (auto& [identifier, expr] : t_csv) {
		copied_symbol_table.add_identifier(identifier, expr->type_info);
	}

	return new TypedFnStatement(
		statement->identifier,
		t_csv,
		type_expression(statement->type, symbol_table),
		type_scope_statement(statement->body, &copied_symbol_table, false) // dont let scope copy as we already did for the params
	);
}
TypedLoopStatement* TypeChecker::type_loop_statement(LoopStatement* statement, SymbolTable* symbol_table) {
	panic("Not implemented");
	return nullptr;
}
TypedStructStatement* TypeChecker::type_struct_statement(StructStatement* statement, SymbolTable* symbol_table) {
	panic("Not implemented");
	return nullptr;
}
TypedAssigmentStatement* TypeChecker::type_assigment_statement(AssigmentStatement* statement, SymbolTable* symbol_table) {
	panic("Not implemented");
	return nullptr;
}
TypedExpressionStatement* TypeChecker::type_expression_statement(ExpressionStatement* statement, SymbolTable* symbol_table) {
	panic("Not implemented");
	return nullptr;
}


TypedExpression* TypeChecker::type_expression(Expression* expression, SymbolTable* symbol_table) {

	{
		auto ptr = dynamic_cast<StringLiteralExpression*>(expression);
		if (ptr) {
			return type_string_literal_expression(ptr, symbol_table);
		}
	}

	{
		auto ptr = dynamic_cast<IntLiteralExpression*>(expression);
		if (ptr) {
			return type_int_literal_expression(ptr, symbol_table);
		}
	}

	{
		auto ptr = dynamic_cast<CallExpression*>(expression);
		if (ptr) {
			return type_call_expression(ptr, symbol_table);
		}
	}

	{
		auto ptr = dynamic_cast<IdentifierExpression*>(expression);
		if (ptr) {
			return type_identifier_expression(ptr, symbol_table);
		}
	}

	{
		auto ptr = dynamic_cast<BinaryExpression*>(expression);
		if (ptr) {
			return type_binary_expression(ptr, symbol_table);
		}
	}

	{
		auto ptr = dynamic_cast<UnaryExpression*>(expression);
		if (ptr) {
			return type_unary_expression(ptr, symbol_table);
		}
	}

	{
		auto ptr = dynamic_cast<GetExpression*>(expression);
		if (ptr) {
			return type_get_expression(ptr, symbol_table);
		}
	}

	{
		auto ptr = dynamic_cast<NewExpression*>(expression);
		if (ptr) {
			return type_new_expression(ptr, symbol_table);
		}
	}

	panic("Not implemented");
	return nullptr;
}
TypedBinaryExpression* TypeChecker::type_binary_expression(BinaryExpression* expression, SymbolTable* symbol_table) {
	panic("Not implemented");
	return nullptr;
}
TypedStringLiteralExpression* TypeChecker::type_string_literal_expression(StringLiteralExpression* expression, SymbolTable* symbol_table) {
	panic("Not implemented");
	return nullptr;
}
TypedIntLiteralExpression* TypeChecker::type_int_literal_expression(IntLiteralExpression* expression, SymbolTable* symbol_table) {
	return new TypedIntLiteralExpression(expression->token);
}
TypedUnaryExpression* TypeChecker::type_unary_expression(UnaryExpression* expression, SymbolTable* symbol_table) {
	panic("Not implemented");
	return nullptr;
}
TypedCallExpression* TypeChecker::type_call_expression(CallExpression* expression, SymbolTable* symbol_table) {
	panic("Not implemented");
	return nullptr;
}
TypedIdentifierExpression* TypeChecker::type_identifier_expression(IdentifierExpression* expression, SymbolTable* symbol_table) {
	TypeInfo type_info;
	
	if (symbol_table->builtin_type_table.contains(expression->identifier.string)) {
		type_info = symbol_table->builtin_type_table[expression->identifier.string];
	} 
	else if (symbol_table->type_table.contains(expression->identifier.string)) {
		type_info = symbol_table->type_table[expression->identifier.string];
	} 
	else if (symbol_table->identifier_table.contains(expression->identifier.string)) {
		type_info = symbol_table->identifier_table[expression->identifier.string];
	}
	else {
		panic("Unrecognized identifier: " + expression->identifier.string);
	}

	return new TypedIdentifierExpression(expression->identifier, type_info);
}

TypedGetExpression* TypeChecker::type_get_expression(GetExpression* expression, SymbolTable* symbol_table) {
	panic("Not implemented");
	return nullptr;
}
TypedNewExpression* TypeChecker::type_new_expression(NewExpression* expression, SymbolTable* symbol_table) {
	panic("Not implemented");
	return nullptr;
}

bool type_match(TypeInfo* a, TypeInfo* b) {
	// TODO: this is temp will need to change
	return a->type == b->type;
}