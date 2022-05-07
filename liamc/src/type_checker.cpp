#include "type_checker.h"
#include "parser.h"
#include <cassert>  

SymbolTable::SymbolTable() {
	this->builtin_type_table = std::map<std::string, TypeInfo*>();
	this->type_table = std::map<std::string, TypeInfo*>();
	this->identifier_table = std::map<std::string, TypeInfo*>();

	builtin_type_table["void"] = new VoidTypeInfo{VOID};
	builtin_type_table["string"] = new StringTypeInfo{STRING};
	builtin_type_table["u64"] = new IntTypeInfo{INT, false, 64};
}

void SymbolTable::add_type(Token type, TypeInfo* type_info) {
	if (type_table.contains(type.string)) {
		panic("Duplcate creation of type: " + type.string + " at (" + std::to_string(type.line) + "," + std::to_string(type.character) + ")");
	}

	type_table[type.string] = type_info;
}

void SymbolTable::add_identifier(Token identifier, TypeInfo* type_info) {
	if (identifier_table.contains(identifier.string)) {
		panic("Duplcate creation of identifier: " + identifier.string + " at (" + std::to_string(identifier.line) + "," + std::to_string(identifier.character) + ")");
	}

	identifier_table[identifier.string] = type_info;
}

void TypedStatement::print() {} // virtual function to keep compiler happy :^]

TypedLetStatement::TypedLetStatement(Token identifier, TypedExpression* type_expression, TypedExpression* expression) {
	this->identifier = identifier;
	this->type_expression = type_expression;
	this->expression = expression;
	this->statement_type = STATEMENT_LET;
}

TypedScopeStatement::TypedScopeStatement(std::vector<TypedStatement*> statements) {
	this->statements = statements;
	this->statement_type = STATEMENT_SCOPE;
}

TypedStructStatement::TypedStructStatement(Token identifier, Typed_CSV members) {
	this->identifier = identifier;
	this->members= members;
	this->statement_type = STATEMENT_SCOPE;
}

TypedFnStatement::TypedFnStatement(Token identifier, Typed_CSV params, TypedExpression* return_type, TypedScopeStatement* body) {
	this->identifier = identifier;
	this->params = params;
	this->return_type = return_type;
	this->body = body;
	this->statement_type = STATEMENT_FN;
}

TypedLoopStatement::TypedLoopStatement(Token identifier, TypedScopeStatement* body) {
	this->identifier = identifier;
	this->body = body;
	this->statement_type = STATEMENT_LOOP;
}

TypedInsertStatement::TypedInsertStatement(TypedExpression* code) {
	this->code = code;
	this->statement_type = STATEMENT_INSERT;
}

TypedReturnStatement::TypedReturnStatement(TypedExpression* epxression) {
	this->expression = expression;
	this->statement_type = STATEMENT_RETURN;
}

TypedBreakStatement::TypedBreakStatement(Token identifier) {
	this->identifier = identifier;
	this->statement_type = STATEMENT_BREAK;
}

TypedImportStatement::TypedImportStatement(TypedExpression* file) {
	this->file = file;
	this->statement_type = STATEMENT_BREAK;
}

TypedExpressionStatement::TypedExpressionStatement(TypedExpression* expression) {
	this->expression = expression;
	this->statement_type = STATEMENT_EXPRESSION;
}

TypedAssigmentStatement::TypedAssigmentStatement(Token identifier, TypedExpression* assigned_to) {
	this->identifier = identifier;
	this->assigned_to = assigned_to;
	this->statement_type = STATEMENT_ASSIGNMENT;
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
	this->type_info = new IntTypeInfo{INT, false, 64};
}

TypedCallExpression::TypedCallExpression(TypedExpression* identifier, std::vector<TypedExpression*> args) {
	this->identifier = identifier;
	this->args = args;
}

TypedIdentifierExpression::TypedIdentifierExpression(Token identifier, TypeInfo* type_info) {
	this->identifier = identifier;
	this->type_info = type_info;
}

TypedStringLiteralExpression::TypedStringLiteralExpression(Token token) {
	this->token = token;
	this->type_info = new StringTypeInfo{ STRING };
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
		auto p = type_statement(stmt, &symbol_table);
		root.statements.push_back(p);
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
		auto ptr = dynamic_cast<ImportStatement*>(statement);
		if (ptr) {
			return type_import_statement(ptr, symbol_table);
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
	auto expression = type_expression(statement->byte_code, symbol_table);
	if (expression->type_info->type != STRING) {
		panic("Insert requires a string");
	}

	return new TypedInsertStatement(expression);
}

TypedReturnStatement* TypeChecker::type_return_statement(ReturnStatement* statement, SymbolTable* symbol_table) {
	panic("Not implemented");
	return nullptr;
}

TypedBreakStatement* TypeChecker::type_break_statement(BreakStatement* statement, SymbolTable* symbol_table) {
	panic("Not implemented");
	return nullptr;
}

TypedImportStatement* TypeChecker::type_import_statement(ImportStatement* statement, SymbolTable* symbol_table) {
	auto expression = type_expression(statement->file, symbol_table);
	if (expression->type_info->type != STRING) {
		panic("Import requires a string");
	}

	return new TypedImportStatement(expression);
}

TypedLetStatement* TypeChecker::type_let_statement(LetStatement* statement, SymbolTable* symbol_table) {
	auto typed_type_expression = type_expression(statement->type, symbol_table);
	auto typed_expression = type_expression(statement->expression, symbol_table);

	if (!type_match(typed_type_expression->type_info, typed_expression->type_info)) {
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

	auto param_type_infos = std::vector<TypeInfo*>();
	Typed_CSV t_csv = Typed_CSV();
	for (auto& [identifier, expr] : statement->params) {
		auto p = type_expression(expr, symbol_table);
		param_type_infos.push_back(p->type_info);
		t_csv.push_back({identifier, p});
	}

	auto return_expresion = type_expression(statement->type, symbol_table);

	symbol_table->add_identifier(statement->identifier, new FnTypeInfo{FN, return_expresion->type_info, param_type_infos});

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
	return new TypedLoopStatement(statement->identifier, type_scope_statement(statement->body, symbol_table));
}

TypedStructStatement* TypeChecker::type_struct_statement(StructStatement* statement, SymbolTable* symbol_table) {
	
	auto members = Typed_CSV();
	auto members_type_info = std::vector<std::tuple<std::string, TypeInfo*>>();
	for (auto& [member, expr] : statement->members) {
		auto expression = type_expression(expr, symbol_table);
		members.push_back({member, expression});
		members_type_info.push_back({member.string, expression->type_info});
	}

	symbol_table->add_type(statement->identifier, new StructTypeInfo{STRUCT, members_type_info});

	return new TypedStructStatement(statement->identifier, members);
}
TypedAssigmentStatement* TypeChecker::type_assigment_statement(AssigmentStatement* statement, SymbolTable* symbol_table) {

	if (!symbol_table->identifier_table.contains(statement->identifier.string)) {
		panic("Trying to assign un-declared identifier");
	}

	auto typed_expression = type_expression(statement->assigned_to->expression, symbol_table);

	if (!type_match(symbol_table->identifier_table[statement->identifier.string], typed_expression->type_info)) {
		panic("Type mismatch, trying to assign a identifier to an expression of different type");
	}

	return new TypedAssigmentStatement(statement->identifier, typed_expression);
}
TypedExpressionStatement* TypeChecker::type_expression_statement(ExpressionStatement* statement, SymbolTable* symbol_table) {
	return new  TypedExpressionStatement(type_expression(statement->expression, symbol_table));
}

void TypedExpression::print() {} // virtual function to keep compiler happy :^]

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
	auto left = type_expression(expression->left, symbol_table);
	auto right = type_expression(expression->right, symbol_table);

	if (!type_match(left->type_info, right->type_info)) {
		panic("Type mismatch in binary expression");
		return nullptr;
	}

	if (left->type_info->type != INT) {
		panic("Cannot use binary operator on non int type");
		return nullptr;	
	}

	auto binary = new TypedBinaryExpression(left, expression->op, right);
	binary->type_info = left->type_info;
	return binary;

}
TypedStringLiteralExpression* TypeChecker::type_string_literal_expression(StringLiteralExpression* expression, SymbolTable* symbol_table) {
	auto typed_expression = new TypedStringLiteralExpression(expression->token);
	typed_expression->type_info = symbol_table->builtin_type_table["string"];
	return typed_expression;
}

TypedIntLiteralExpression* TypeChecker::type_int_literal_expression(IntLiteralExpression* expression, SymbolTable* symbol_table) {
	return new TypedIntLiteralExpression(expression->token);
}
TypedUnaryExpression* TypeChecker::type_unary_expression(UnaryExpression* expression, SymbolTable* symbol_table) {
	auto typed_expression = type_expression(expression->expression, symbol_table);

	TypeInfo* type_info;
	if (expression->op.type == TOKEN_AT || expression->op.type == TOKEN_HAT) {
		type_info = new PointerTypeInfo{ POINTER, typed_expression->type_info };
	}
	else if(expression->op.type == TOKEN_STAR) {
		if (typed_expression->type_info->type != POINTER) {
			panic("Cannot dereference non-pointer value");
		}

		auto ptr_type = static_cast<PointerTypeInfo*>(typed_expression->type_info);
		type_info = ptr_type->to;
	}
	else {
		panic("Unrecognised unary operator");
		return nullptr;
	}
	
	auto typed_unary = new TypedUnaryExpression(typed_expression, expression->op);
	typed_unary->type_info = type_info;
	return typed_unary;
}
TypedCallExpression* TypeChecker::type_call_expression(CallExpression* expression, SymbolTable* symbol_table) {
	auto type_of_callee = type_expression(expression->identifier, symbol_table);

	auto typed_args = std::vector<TypedExpression*>();
	auto arg_type_infos = std::vector<TypeInfo*>();
	for (auto arg : expression->args) {
		auto typed_arg = type_expression(arg, symbol_table);
		typed_args.push_back(typed_arg);
		arg_type_infos.push_back(typed_arg->type_info);
	}

	if (type_of_callee->type_info->type != FN) {
		panic("Can only call a function");
	}

	auto fn_type_info = static_cast<FnTypeInfo*>(type_of_callee->type_info);
	if (fn_type_info->args.size() != arg_type_infos.size()) {
		panic("Incorrect numbers of args passed to function call");
	}

	for (int i = 0; i < fn_type_info->args.size(); i++) {
		if (!type_match(fn_type_info->args.at(i), arg_type_infos.at(i))) {
			panic("Type mismatch at function call");
		}
	}

	auto return_type = fn_type_info->return_type;

	auto final_expression = new TypedCallExpression(type_of_callee, typed_args);
	final_expression->type_info = return_type;
	return final_expression;
}
TypedIdentifierExpression* TypeChecker::type_identifier_expression(IdentifierExpression* expression, SymbolTable* symbol_table) {
	TypeInfo* type_info;
	
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
	auto typed_expression = type_expression(expression->expression, symbol_table);
	if (typed_expression->type_info->type != STRUCT) {
		panic("Cannot derive member from non struct type");
	}

	TypeInfo* member_type_info = nullptr;
	auto struct_type_info = static_cast<StructTypeInfo*>(typed_expression->type_info);
	for (auto [identifier, member_type] : struct_type_info->members) {
		if (identifier == expression->member.string) {
			member_type_info = member_type;
		}
	}

	if (member_type_info == nullptr) {
		panic("Cannot find member in struct");
	}

	auto final_expression = new TypedGetExpression(typed_expression, expression->member);
	final_expression->type_info = member_type_info;
	return final_expression;
}


TypedNewExpression* TypeChecker::type_new_expression(NewExpression* expression, SymbolTable* symbol_table) {
	
	// check its a type
	if (!symbol_table->type_table.contains(expression->identifier.string)) {
		panic("Unrecognised type in new statement");
	}

	// check its a struct
	if (symbol_table->type_table[expression->identifier.string]->type != STRUCT) {
		panic("Cannot use new on non struct identifier");
	}

	auto struct_type_info = static_cast<StructTypeInfo*>(symbol_table->type_table[expression->identifier.string]);

	// collect members from new constructor
	auto typed_expressions = std::vector<TypedExpression*>();
	auto members_type_info = std::vector<TypeInfo*>();
	for (auto expr : expression->expressions) {
		auto typed_member = type_expression(expr, symbol_table);
		typed_expressions.push_back(typed_member);
		members_type_info.push_back(typed_member->type_info);
	}

	// check counts
	if (struct_type_info->members.size() != members_type_info.size()) {
		panic("Incorrect number of arguments in new constructor");
	}

	// check types
	for (int i = 0; i < members_type_info.size(); i++) {
		auto [member, type] = struct_type_info->members.at(i);
		if (!type_match(members_type_info.at(i), type)) {
			panic("Incorect arguments to new constructor");
		}
	}

	auto typed_expression = new TypedNewExpression(expression->identifier, typed_expressions);
	typed_expression->type_info = struct_type_info;

	return typed_expression;
}

bool type_match(TypeInfo* a, TypeInfo* b) {
	if (a->type != b->type) {
		return false;
	}

	if (a->type == VOID || a->type == STRING) {
		return true;
	}
	else if (a->type == INT) {
		auto int_a = static_cast<IntTypeInfo*>(a);
		auto int_b = static_cast<IntTypeInfo*>(b);

		if (int_a->is_signed == int_b->is_signed && int_a->size == int_b->size)
			return true;

		return false;
	}
	else if (a->type == FN) {
		auto fn_a = static_cast<FnTypeInfo*>(a);
		auto fn_b = static_cast<FnTypeInfo*>(b);

		if (type_match(fn_a->return_type, fn_b->return_type)) {
			for (int i = 0; i < fn_a->args.size(); i++) {
				if(type_match(fn_a->args.at(i), fn_b->args.at(i))) {}
				else { return false;  }
			}

			return true;
		}

		return false;
	}
	else if (a->type == STRUCT) {
		auto struct_a = static_cast<StructTypeInfo*>(a);
		auto struct_b = static_cast<StructTypeInfo*>(b);

		for (int i = 0; i < struct_a->members.size(); i++) {
			auto [a_member, a_type] = struct_a->members.at(i);
			auto [b_member, b_type] = struct_b->members.at(i);
			if (!type_match(a_type, b_type)) {
				return false;
			}
		}

		return true;
	}
	else if (a->type == POINTER) {
		auto ptr_a = static_cast<PointerTypeInfo*>(a);
		auto ptr_b = static_cast<PointerTypeInfo*>(b);

		if (ptr_a->to->type == VOID) return true; // void^ can be equal to T^, not other way around
		return ptr_a->to->type == ptr_b->to->type;
	}
}