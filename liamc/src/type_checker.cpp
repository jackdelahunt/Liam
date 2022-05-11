#include "type_checker.h"
#include "parser.h"
#include <cassert>  

SymbolTable::
SymbolTable() {
	this->builtin_type_table = std::map<std::string, TypeInfo*>();
	this->type_table = std::map<std::string, TypeInfo*>();
	this->identifier_table = std::map<std::string, TypeInfo*>();

	builtin_type_table["void"] = new VoidTypeInfo{VOID};
	builtin_type_table["string"] = new StringTypeInfo{STRING};
	builtin_type_table["u64"] = new IntTypeInfo{INT, false, 64};
}

void SymbolTable::
add_type(Token type, TypeInfo* type_info) {
	if (type_table.contains(type.string)) {
		panic("Duplcate creation of type: " + type.string + " at (" + std::to_string(type.line) + "," + std::to_string(type.character) + ")");
	}

	type_table[type.string] = type_info;
}

void SymbolTable::
add_identifier(Token identifier, TypeInfo* type_info) {
	if (identifier_table.contains(identifier.string)) {
		panic("Duplcate creation of identifier: " + identifier.string + " at (" + std::to_string(identifier.line) + "," + std::to_string(identifier.character) + ")");
	}

	identifier_table[identifier.string] = type_info;
}

TypeInfo* SymbolTable::
get_type(Token* identifier) {
    if(builtin_type_table.contains(identifier->string)) {
        return builtin_type_table[identifier->string];
    }

    if(identifier_table.contains(identifier->string)) {
        return identifier_table[identifier->string];
    }

    if(type_table.contains(identifier->string)) {
        return type_table[identifier->string];
    }

    return nullptr;
}

void TypeCheckedStatement::
print() {} // virtual function to keep compiler happy :^]

TypeCheckedLetStatement::
TypeCheckedLetStatement(Token identifier, TypeCheckedTypeExpression* type_expression, TypeCheckedExpression* expression) {
	this->identifier = identifier;
	this->type_expression = type_expression;
	this->expression = expression;
	this->statement_type = STATEMENT_LET;
}

TypeCheckedScopeStatement::
TypeCheckedScopeStatement(std::vector<TypeCheckedStatement*> statements) {
	this->statements = statements;
	this->statement_type = STATEMENT_SCOPE;
}

TypeCheckedStructStatement::
TypeCheckedStructStatement(Token identifier, Typed_CSV members) {
	this->identifier = identifier;
	this->members= members;
	this->statement_type = STATEMENT_SCOPE;
}

TypeCheckedFnStatement::
TypeCheckedFnStatement(Token identifier, Typed_CSV params, TypeCheckedTypeExpression* return_type, TypeCheckedScopeStatement* body) {
	this->identifier = identifier;
	this->params = params;
	this->return_type = return_type;
	this->body = body;
	this->statement_type = STATEMENT_FN;
}

TypeCheckedLoopStatement::
TypeCheckedLoopStatement(Token identifier, TypeCheckedScopeStatement* body) {
	this->identifier = identifier;
	this->body = body;
	this->statement_type = STATEMENT_LOOP;
}

TypeCheckedInsertStatement::
TypeCheckedInsertStatement(TypeCheckedStringLiteralExpression* code) {
	this->code = code;
	this->statement_type = STATEMENT_INSERT;
}

TypeCheckedReturnStatement::
TypeCheckedReturnStatement(TypeCheckedExpression* expression) {
	this->expression = expression;
	this->statement_type = STATEMENT_RETURN;
}

TypeCheckedBreakStatement::TypeCheckedBreakStatement(Token identifier) {
	this->identifier = identifier;
	this->statement_type = STATEMENT_BREAK;
}

TypeCheckedImportStatement::
TypeCheckedImportStatement(TypeCheckedExpression* file) {
	this->file = file;
	this->statement_type = STATEMENT_BREAK;
}

TypeCheckedExpressionStatement::
TypeCheckedExpressionStatement(TypeCheckedExpression* expression) {
	this->expression = expression;
	this->statement_type = STATEMENT_EXPRESSION;
}

TypeCheckedAssigmentStatement::
TypeCheckedAssigmentStatement(Token identifier, TypeCheckedExpression* assigned_to) {
	this->identifier = identifier;
	this->assigned_to = assigned_to;
	this->statement_type = STATEMENT_ASSIGNMENT;
}

TypeCheckedBinaryExpression::
TypeCheckedBinaryExpression(TypeCheckedExpression* left, Token op, TypeCheckedExpression* right) {
	this->left = left;
	this->op = op;
	this->right = right;
    this->type = ExpressionType::EXPRESSION_BINARY;
}

TypeCheckedUnaryExpression::
TypeCheckedUnaryExpression(TypeCheckedExpression* expression, Token op) {
	this->expression = expression;
	this->op = op;
    this->type = ExpressionType::EXPRESSION_UNARY;
}

TypeCheckedIntLiteralExpression::
TypeCheckedIntLiteralExpression(Token token) {
	this->token = token;
	this->type_info = new IntTypeInfo{INT, false, 64};
    this->type = ExpressionType::EXPRESSION_INT_LITERAL;
}

TypeCheckedCallExpression::
TypeCheckedCallExpression(TypeCheckedIdentifierExpression* identifier, std::vector<TypeCheckedExpression*> args) {
	this->identifier = identifier;
	this->args = args;
    this->type = ExpressionType::EXPRESSION_CALL;
}

TypeCheckedIdentifierExpression::
TypeCheckedIdentifierExpression(Token identifier, TypeInfo* type_info) {
    this->identifier = identifier;
    this->type_info = type_info;
    this->type = ExpressionType::EXPRESSION_IDENTIFIER;
}

TypeCheckedStringLiteralExpression::
TypeCheckedStringLiteralExpression(Token token) {
	this->token = token;
	this->type_info = new StringTypeInfo{ STRING };
    this->type = ExpressionType::EXPRESSION_STRING_LITERAL;
}


TypeCheckedGetExpression::
TypeCheckedGetExpression(TypeCheckedExpression* expression, Token member) {
	this->expression = expression;
	this->member = member;
    this->type = ExpressionType::EXPRESSION_GET;
}

TypeCheckedNewExpression::
TypeCheckedNewExpression(Token identifier, std::vector<TypeCheckedExpression*> expressions) {
	this->identifier = identifier;
	this->expressions = expressions;
    this->type = ExpressionType::EXPRESSION_NEW;
}

TypeCheckedArrayExpression::
TypeCheckedArrayExpression(std::vector<TypeCheckedExpression*> expressions) {
    this->expressions = std::move(expressions);
    this->type = ExpressionType::EXPRESSION_ARRAY;
}

void TypeCheckedTypeExpression::
print() {} // virtual function to keep compiler happy :^]

TypeCheckedIdentifierTypeExpression::TypeCheckedIdentifierTypeExpression(Token identifier) {
    this->identifier = identifier;
    this->type = TypeExpressionType::TYPE_IDENTIFIER;
}

TypeCheckedPointerTypeExpression::
TypeCheckedPointerTypeExpression(TypeCheckedTypeExpression* pointer_of) {
    this->pointer_of = pointer_of;
    this->type = TypeExpressionType::TYPE_POINTER;
}

TypeChecker::
TypeChecker() {
	symbol_table = SymbolTable();
	root = TypedFile();
}

void TypeChecker::
type_file(File* file) {
	for (auto stmt : file->statements) {
		auto p = type_check_statement(stmt, &symbol_table);
		root.statements.push_back(p);
	}
}

TypeCheckedStatement* TypeChecker::
type_check_statement(Statement* statement, SymbolTable* symbol_table) {
    switch (statement->statement_type) {
        case STATEMENT_INSERT: return type_check_insert_statement(dynamic_cast<InsertStatement *>(statement),
                                                                  symbol_table); break;
        case STATEMENT_RETURN: return type_check_return_statement(dynamic_cast<ReturnStatement *>(statement),
                                                                  symbol_table); break;
        case STATEMENT_BREAK: return type_check_break_statement(dynamic_cast<BreakStatement *>(statement), symbol_table); break;
        case STATEMENT_IMPORT: return type_check_import_statement(dynamic_cast<ImportStatement *>(statement),
                                                                  symbol_table); break;
        case STATEMENT_FN: return type_check_fn_statement(dynamic_cast<FnStatement *>(statement), symbol_table); break;
        case STATEMENT_LOOP: return type_check_loop_statement(dynamic_cast<LoopStatement *>(statement), symbol_table); break;
        case STATEMENT_STRUCT: return type_check_struct_statement(dynamic_cast<StructStatement *>(statement),
                                                                  symbol_table); break;
        case STATEMENT_ASSIGNMENT: return type_check_assigment_statement(dynamic_cast<AssigmentStatement *>(statement),
                                                                         symbol_table); break;
        case STATEMENT_EXPRESSION: return type_check_expression_statement(
                    dynamic_cast<ExpressionStatement *>(statement), symbol_table); break;
        case STATEMENT_LET: return type_check_let_statement(dynamic_cast<LetStatement *>(statement), symbol_table); break;
        default:
            panic("Not implemented");
            return nullptr;
    }
}

TypeCheckedInsertStatement* TypeChecker::
type_check_insert_statement(InsertStatement* statement, SymbolTable* symbol_table) {
	auto expression = type_check_expression(statement->byte_code, symbol_table);
	if (expression->type_info->type != STRING) {
		panic("Insert requires a string");
	}

	return new TypeCheckedInsertStatement(dynamic_cast<TypeCheckedStringLiteralExpression*>(expression));
}

TypeCheckedReturnStatement* TypeChecker::
type_check_return_statement(ReturnStatement* statement, SymbolTable* symbol_table) {
    return new TypeCheckedReturnStatement(type_check_expression(statement->expression, symbol_table));
}

TypeCheckedBreakStatement* TypeChecker::
type_check_break_statement(BreakStatement* statement, SymbolTable* symbol_table) {
	panic("Not implemented");
	return nullptr;
}

TypeCheckedImportStatement* TypeChecker::
type_check_import_statement(ImportStatement* statement, SymbolTable* symbol_table) {
	auto expression = type_check_expression(statement->file, symbol_table);
	if (expression->type_info->type != STRING) {
		panic("Import requires a string");
	}

	return new TypeCheckedImportStatement(expression);
}

TypeCheckedLetStatement* TypeChecker::
type_check_let_statement(LetStatement* statement, SymbolTable* symbol_table) {
    TypeCheckedTypeExpression* let_type = nullptr;
    auto typed_expression = type_check_expression(statement->expression, symbol_table);
    if(statement->type) {
        let_type = type_check_type_expression(statement->type, symbol_table);
        if (!type_match(let_type->type_info, typed_expression->type_info)) {
            panic("Mis matched types in let statement");
        }
    }

	symbol_table->add_identifier(statement->identifier, typed_expression->type_info);

	return new TypeCheckedLetStatement(
		statement->identifier,
		let_type,
		typed_expression
	);
}
TypeCheckedScopeStatement* TypeChecker::
type_check_scope_statement(ScopeStatement* statement, SymbolTable* symbol_table, bool copy_symbol_table) {
	auto statements = std::vector<TypeCheckedStatement*>();

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
		statements.push_back(type_check_statement(stmt, scopes_symbol_table));
	}

	return new TypeCheckedScopeStatement(statements);
}
TypeCheckedFnStatement* TypeChecker::
type_check_fn_statement(FnStatement* statement, SymbolTable* symbol_table) {

	auto param_type_infos = std::vector<TypeInfo*>();
	Typed_CSV t_csv = Typed_CSV();
	for (auto& [identifier, expr] : statement->params) {
		auto p = type_check_type_expression(expr, symbol_table);
		param_type_infos.push_back(p->type_info);
		t_csv.push_back({identifier, p});
	}

	auto return_expression = type_check_type_expression(statement->type, symbol_table);

	symbol_table->add_identifier(statement->identifier, new FnTypeInfo{FN, return_expression->type_info, param_type_infos});

	SymbolTable copied_symbol_table = *symbol_table;
	for (auto& [identifier, expr] : t_csv) {
		copied_symbol_table.add_identifier(identifier, expr->type_info);
	}

    // type body and check return
    auto typed_body = type_check_scope_statement(statement->body, &copied_symbol_table, false);
    if(return_expression->type_info->type == VOID) {
        for(auto stmt : typed_body->statements) {
            if(stmt->statement_type == STATEMENT_RETURN) {
                panic("found return statement when return type is void");
            }
        }
    } else {
        bool found_return = false;
        for(auto stmt : typed_body->statements) {
            if (stmt->statement_type == STATEMENT_RETURN) {
                found_return = true;
                auto return_statement = dynamic_cast<TypeCheckedReturnStatement*>(stmt);
                if(!type_match(return_statement->expression->type_info, return_expression->type_info)) {
                    panic("Mismatch types in function, return types do not match");
                }
            }
        }

        if(!found_return) {
            panic("No return statement in function that has non void return type");
        }
    }

	return new TypeCheckedFnStatement(
            statement->identifier,
            t_csv,
            type_check_type_expression(statement->type, symbol_table),
            typed_body
	);
}

TypeCheckedLoopStatement* TypeChecker::
type_check_loop_statement(LoopStatement* statement, SymbolTable* symbol_table) {
	return new TypeCheckedLoopStatement(statement->identifier, type_check_scope_statement(statement->body, symbol_table));
}

TypeCheckedStructStatement* TypeChecker::
type_check_struct_statement(StructStatement* statement, SymbolTable* symbol_table) {
	
	auto members = Typed_CSV();
	auto members_type_info = std::vector<std::tuple<std::string, TypeInfo*>>();
	for (auto& [member, expr] : statement->members) {
		auto expression = type_check_type_expression(expr, symbol_table);
		members.push_back({member, expression});
		members_type_info.push_back({member.string, expression->type_info});
	}

	symbol_table->add_type(statement->identifier, new StructTypeInfo{STRUCT, members_type_info});

	return new TypeCheckedStructStatement(statement->identifier, members);
}
TypeCheckedAssigmentStatement* TypeChecker::
type_check_assigment_statement(AssigmentStatement* statement, SymbolTable* symbol_table) {

	if (!symbol_table->identifier_table.contains(statement->identifier.string)) {
		panic("Trying to assign un-declared identifier");
	}

	auto typed_expression = type_check_expression(statement->assigned_to->expression, symbol_table);

	if (!type_match(symbol_table->identifier_table[statement->identifier.string], typed_expression->type_info)) {
		panic("Type mismatch, trying to assign a identifier to an expression of different type");
	}

	return new TypeCheckedAssigmentStatement(statement->identifier, typed_expression);
}
TypeCheckedExpressionStatement* TypeChecker::
type_check_expression_statement(ExpressionStatement* statement, SymbolTable* symbol_table) {
	return new  TypeCheckedExpressionStatement(type_check_expression(statement->expression, symbol_table));
}

void TypeCheckedExpression::print() {} // virtual function to keep compiler happy :^]

TypeCheckedExpression* TypeChecker::
type_check_expression(Expression* expression, SymbolTable* symbol_table) {
    switch (expression->type) {
        case ExpressionType::EXPRESSION_STRING_LITERAL:
            return type_check_string_literal_expression(dynamic_cast<StringLiteralExpression *>(expression),
                                                        symbol_table); break;
        case ExpressionType::EXPRESSION_INT_LITERAL:
            return type_check_int_literal_expression(dynamic_cast<IntLiteralExpression *>(expression), symbol_table); break;
        case ExpressionType::EXPRESSION_CALL:
            return type_check_call_expression(dynamic_cast<CallExpression*>(expression), symbol_table); break;
        case ExpressionType::EXPRESSION_IDENTIFIER:
            return type_check_identifier_expression(dynamic_cast<IdentifierExpression*>(expression), symbol_table); break;
        case ExpressionType::EXPRESSION_BINARY:
            return type_check_binary_expression(dynamic_cast<BinaryExpression *>(expression), symbol_table); break;
        case ExpressionType::EXPRESSION_UNARY:
            return type_check_unary_expression(dynamic_cast<UnaryExpression *>(expression), symbol_table); break;
        case ExpressionType::EXPRESSION_GET:
            return type_check_get_expression(dynamic_cast<GetExpression*>(expression), symbol_table); break;
        case ExpressionType::EXPRESSION_NEW:
            return type_check_new_expression(dynamic_cast<NewExpression*>(expression), symbol_table); break;
        case ExpressionType::EXPRESSION_ARRAY:
            return type_check_array_expression(dynamic_cast<ArrayExpression*>(expression), symbol_table); break;
        default:
            panic("Not implemented");
            return nullptr;
    }
}
TypeCheckedBinaryExpression* TypeChecker::
type_check_binary_expression(BinaryExpression* expression, SymbolTable* symbol_table) {
	auto left = type_check_expression(expression->left, symbol_table);
	auto right = type_check_expression(expression->right, symbol_table);

	if (!type_match(left->type_info, right->type_info)) {
		panic("Type mismatch in binary expression");
		return nullptr;
	}

	if (left->type_info->type != INT) {
		panic("Cannot use binary operator on non int type");
		return nullptr;	
	}

	auto binary = new TypeCheckedBinaryExpression(left, expression->op, right);
	binary->type_info = left->type_info;
	return binary;

}
TypeCheckedStringLiteralExpression* TypeChecker::
type_check_string_literal_expression(StringLiteralExpression* expression, SymbolTable* symbol_table) {
	auto typed_expression = new TypeCheckedStringLiteralExpression(expression->token);
	typed_expression->type_info = symbol_table->builtin_type_table["string"];
	return typed_expression;
}

TypeCheckedIntLiteralExpression* TypeChecker::
type_check_int_literal_expression(IntLiteralExpression* expression, SymbolTable* symbol_table) {
	return new TypeCheckedIntLiteralExpression(expression->token);
}
TypeCheckedUnaryExpression* TypeChecker::
type_check_unary_expression(UnaryExpression* expression, SymbolTable* symbol_table) {
	auto typed_expression = type_check_expression(expression->expression, symbol_table);

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
    else if(expression->op.type == TOKEN_RANGE) {
        type_info = new ArrayTypeInfo{ARRAY, typed_expression->type_info};
    }
	else {
		panic("Unrecognised unary operator");
		return nullptr;
	}
	
	auto typed_unary = new TypeCheckedUnaryExpression(typed_expression, expression->op);
	typed_unary->type_info = type_info;
	return typed_unary;
}
TypeCheckedCallExpression* TypeChecker::
type_check_call_expression(CallExpression* expression, SymbolTable* symbol_table) {
	auto type_of_callee = dynamic_cast<TypeCheckedIdentifierExpression*>(
            type_check_expression(expression->identifier, symbol_table)
            );

    if(!type_of_callee) {
        panic("Can only call on identifier expressions");
    }

	auto typed_args = std::vector<TypeCheckedExpression*>();
	auto arg_type_infos = std::vector<TypeInfo*>();
	for (auto arg : expression->args) {
		auto typed_arg = type_check_expression(arg, symbol_table);
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

	auto final_expression = new TypeCheckedCallExpression(type_of_callee, typed_args);
	final_expression->type_info = return_type;
	return final_expression;
}
TypeCheckedIdentifierExpression* TypeChecker::
type_check_identifier_expression(IdentifierExpression* expression, SymbolTable* symbol_table) {
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

	return new TypeCheckedIdentifierExpression(expression->identifier, type_info);
}

TypeCheckedGetExpression* TypeChecker::
type_check_get_expression(GetExpression* expression, SymbolTable* symbol_table) {
	auto typed_expression = type_check_expression(expression->expression, symbol_table);
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

	auto final_expression = new TypeCheckedGetExpression(typed_expression, expression->member);
	final_expression->type_info = member_type_info;
	return final_expression;
}


TypeCheckedNewExpression* TypeChecker::
type_check_new_expression(NewExpression* expression, SymbolTable* symbol_table) {
	
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
	auto typed_expressions = std::vector<TypeCheckedExpression*>();
	auto members_type_info = std::vector<TypeInfo*>();
	for (auto expr : expression->expressions) {
		auto typed_member = type_check_expression(expr, symbol_table);
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

	auto typed_expression = new TypeCheckedNewExpression(expression->identifier, typed_expressions);
	typed_expression->type_info = struct_type_info;

	return typed_expression;
}

TypeCheckedArrayExpression* TypeChecker::
type_check_array_expression(ArrayExpression* expression, SymbolTable* symbol_table) {
    auto typed_expressions = std::vector<TypeCheckedExpression*>();
    for(auto expr : expression->expressions) {
        auto typed_expression = type_check_expression(expr, symbol_table);
        typed_expressions.emplace_back(typed_expression);
    }

    TypeInfo* array_type = nullptr;
    if(!typed_expressions.empty()) {
        array_type = typed_expressions.at(0)->type_info;
    }

    if(typed_expressions.size() > 1) {
        auto first_type = typed_expressions.at(0)->type_info;
        for(int i = 0; i < typed_expressions.size(); i++) {
            if(!type_match(first_type, typed_expressions.at(i)->type_info)) {
                panic("Mismatched types in array expression, index " + std::to_string(i) + " has unexpected type");
            }
        }
    }

    auto typed_array_expression = new TypeCheckedArrayExpression(typed_expressions);
    typed_array_expression->type_info = new ArrayTypeInfo{ARRAY, array_type};

    return typed_array_expression;
}

TypeCheckedTypeExpression* TypeChecker::
type_check_type_expression(TypeExpression* type_expression, SymbolTable* symbol_table) {
    switch (type_expression->type) {
        case TypeExpressionType::TYPE_IDENTIFIER:
            return type_check_identifier_type_expression(dynamic_cast<IdentifierTypeExpression *>(type_expression), symbol_table); break;
        case TypeExpressionType::TYPE_POINTER:
            return type_check_pointer_type_expression(dynamic_cast<PointerTypeExpression *>(type_expression), symbol_table); break;
    }
}

TypeCheckedIdentifierTypeExpression* TypeChecker::
type_check_identifier_type_expression(IdentifierTypeExpression* type_expression, SymbolTable* symbol_table) {
    auto typed_expression = new TypeCheckedIdentifierTypeExpression(type_expression->identifier);
    auto type = symbol_table->get_type(&type_expression->identifier);
    if(type) {
        typed_expression->type_info = type;
        return typed_expression;
    }

    panic("Unrecognised type in type expression: " + type_expression->identifier.string);
    return nullptr;
}

TypeCheckedPointerTypeExpression* TypeChecker::
type_check_pointer_type_expression(PointerTypeExpression* type_expression, SymbolTable* symbol_table) {
    auto typed_pointer_of = type_check_type_expression(type_expression->pointer_of, symbol_table);
    auto typed_pointer_expression = new TypeCheckedPointerTypeExpression(typed_pointer_of);
    auto info = new PointerTypeInfo{ POINTER, typed_pointer_of->type_info };
    typed_pointer_expression->type_info = info;
    return typed_pointer_expression;
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

    return false;
}