#include "type_checker.h"
#include "parser.h"
#include "liam.h"

SymbolTable::
SymbolTable() {
	this->builtin_type_table = std::map<std::string, TypeInfo*>();
	this->type_table = std::map<std::string, TypeInfo*>();
	this->identifier_table = std::map<std::string, TypeInfo*>();

	builtin_type_table["void"] = new VoidTypeInfo{TypeInfoType::VOID};
    builtin_type_table["char"] = new CharTypeInfo{TypeInfoType::CHAR};
    builtin_type_table["u64"] = new IntTypeInfo{TypeInfoType::INT, false, 64};
    builtin_type_table["bool"] = new BoolTypeInfo{TypeInfoType::BOOL};
}

void SymbolTable::
add_type(Token type, TypeInfo* type_info) {
	if (type_table.contains(type.string)) {
		panic("Duplcate creation of return_type: " + type.string + " at (" + std::to_string(type.line) + "," + std::to_string(type.character) + ")");
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
    return get_type(identifier->string);
}

TypeInfo* SymbolTable::
get_type(std::string identifier) {
    if(builtin_type_table.contains(identifier)) {
        return builtin_type_table[identifier];
    }

    if(identifier_table.contains(identifier)) {
        return identifier_table[identifier];
    }

    if(type_table.contains(identifier)) {
        return type_table[identifier];
    }

    return nullptr;
}

TypeChecker::
TypeChecker() {
	symbol_table = SymbolTable();
}

File TypeChecker::
type_check(std::vector<File>* files) {
    auto typed_file = File("deez nuts");

    auto structs = std::vector<StructStatement*>();
    auto funcs = std::vector<FnStatement*>();
    auto others = std::vector<Statement*>();

    for(auto& file : *files) {
        for (auto stmt: file.statements) {
            if (stmt->statement_type == StatementType::STATEMENT_STRUCT) {
                structs.push_back(dynamic_cast<StructStatement *>(stmt));
            } else if (stmt->statement_type == StatementType::STATEMENT_FN) {
                funcs.push_back(dynamic_cast<FnStatement*>(stmt));
                others.push_back(stmt);
            } else {
                others.push_back(stmt);
            }
        }
    }

    //struct identifier pass
    for(auto stmt : structs) {
        symbol_table.add_type(stmt->identifier, nullptr);
    }

    //struct return_type pass
    for (auto stmt : structs) {
        type_check_struct_statement(stmt, &symbol_table, true);
        typed_file.statements.push_back(stmt);
    }

    // function decl pass
    for (auto stmt : funcs) {
        type_check_fn_decl(stmt, &symbol_table);
    }

    for (auto stmt: others) {
        type_check_statement(stmt, &symbol_table);
        typed_file.statements.push_back(stmt);
    }

    return typed_file;
}

void TypeChecker::
type_check_fn_decl(FnStatement* statement, SymbolTable* symbol_table) {
    auto param_type_infos = std::vector<TypeInfo*>();
    for (auto& [identifier, expr] : statement->params) {
        type_check_type_expression(expr, symbol_table);
        param_type_infos.push_back(expr->type_info);
    }

    type_check_type_expression(statement->return_type, symbol_table);
    symbol_table->add_identifier(statement->identifier, new FnTypeInfo{TypeInfoType::FN, statement->return_type->type_info,param_type_infos});
}

void TypeChecker::
type_check_statement(Statement* statement, SymbolTable* symbol_table) {
    switch (statement->statement_type) {
        case StatementType::STATEMENT_INSERT: return type_check_insert_statement(dynamic_cast<InsertStatement *>(statement),symbol_table); break;
        case StatementType::STATEMENT_RETURN: return type_check_return_statement(dynamic_cast<ReturnStatement *>(statement),symbol_table); break;
        case StatementType::STATEMENT_BREAK: return type_check_break_statement(dynamic_cast<BreakStatement *>(statement), symbol_table); break;
        case StatementType::STATEMENT_FN: return type_check_fn_statement(dynamic_cast<FnStatement *>(statement), symbol_table); break;
        case StatementType::STATEMENT_LOOP: return type_check_loop_statement(dynamic_cast<LoopStatement *>(statement), symbol_table); break;
        case StatementType::STATEMENT_STRUCT: return type_check_struct_statement(dynamic_cast<StructStatement *>(statement),symbol_table); break;
        case StatementType::STATEMENT_ASSIGNMENT: return type_check_assigment_statement(dynamic_cast<AssigmentStatement *>(statement),symbol_table); break;
        case StatementType::STATEMENT_EXPRESSION: return type_check_expression_statement(dynamic_cast<ExpressionStatement *>(statement), symbol_table); break;
        case StatementType::STATEMENT_LET: return type_check_let_statement(dynamic_cast<LetStatement *>(statement), symbol_table); break;
        case StatementType::STATEMENT_FOR: return type_check_for_statement(dynamic_cast<ForStatement *>(statement), symbol_table); break;
        case StatementType::STATEMENT_IF: return type_check_if_statement(dynamic_cast<IfStatement *>(statement), symbol_table); break;
        default:
            panic("Statement not implemented in return_type checker, id -> " + std::to_string((int)statement->statement_type));
    }
}

void TypeChecker::
type_check_insert_statement(InsertStatement* statement, SymbolTable* symbol_table) {
	type_check_expression(statement->byte_code, symbol_table);
    auto to_compare = PointerTypeInfo{TypeInfoType::POINTER, symbol_table->builtin_type_table["char"]};
	if (!type_match(statement->byte_code->type_info, &to_compare)) {
		panic("Insert requires a string");
	}
}

void TypeChecker::
type_check_return_statement(ReturnStatement* statement, SymbolTable* symbol_table) {
    type_check_expression(statement->expression, symbol_table);
}

void TypeChecker::
type_check_break_statement(BreakStatement* statement, SymbolTable* symbol_table) {
	panic("Not implemented");
}

void TypeChecker::
type_check_let_statement(LetStatement* statement, SymbolTable* symbol_table) {
    type_check_expression(statement->rhs, symbol_table);
    if(statement->type) {
        type_check_type_expression(statement->type, symbol_table);
        if (!type_match(statement->type->type_info, statement->rhs->type_info)) {
            panic("Mis matched types in let statement");
        }
    }

    // let type could be null here that is why rhs is added. if there is ever lets
    // with no rhs this will break... :[
    symbol_table->add_identifier(statement->identifier, statement->rhs->type_info);
}
void TypeChecker::
type_check_scope_statement(ScopeStatement* statement, SymbolTable* symbol_table, bool copy_symbol_table) {
	// this is kind of a mess... oh jeez
	SymbolTable* scopes_symbol_table = symbol_table;
	SymbolTable possible_copy;
	if (copy_symbol_table) {
		possible_copy = *symbol_table;
		scopes_symbol_table = &possible_copy;
	}

	for (auto stmt : statement->statements) {
		type_check_statement(stmt, scopes_symbol_table);
	}

}
void TypeChecker::
type_check_fn_statement(FnStatement* statement, SymbolTable* symbol_table, bool first_pass) {
    auto existing_type_info = static_cast<FnTypeInfo*>(symbol_table->identifier_table[statement->identifier.string]);

    // params and get return_type expressions
	CSV t_csv = CSV();
	for (auto& [identifier, expr] : statement->params) {
		type_check_type_expression(expr, symbol_table);
		t_csv.push_back({identifier, expr});
	}

    // copy table and add params
	SymbolTable copied_symbol_table = *symbol_table;
	for (auto& [identifier, expr] : t_csv) {
		copied_symbol_table.add_identifier(identifier, expr->type_info);
	}

    // type statements and check return exists if needed
    type_check_scope_statement(statement->body, &copied_symbol_table, false);
    if(existing_type_info->return_type->type == TypeInfoType::VOID) {
        for(auto stmt : statement->body->statements) {
            if(stmt->statement_type == StatementType::STATEMENT_RETURN) {
                panic("found return statement when return return_type is void");
            }
        }
    } else {
        bool found_return = false;
        for(auto stmt : statement->body->statements) {
            if (stmt->statement_type == StatementType::STATEMENT_RETURN) {
                found_return = true;
                auto return_statement = dynamic_cast<ReturnStatement*>(stmt);
                if(!type_match(return_statement->expression->type_info, existing_type_info->return_type)) {
                    panic("Mismatch types in function, return types do not match");
                }
            }
        }

        if(!found_return) {
            panic("No return statement in function that has non void return return_type");
        }
    }
}

void TypeChecker::
type_check_loop_statement(LoopStatement* statement, SymbolTable* symbol_table) {
	type_check_scope_statement(statement->body, symbol_table);
}

void TypeChecker::
type_check_for_statement(ForStatement *statement, SymbolTable *symbol_table) {
    /*type_check_let_statement(statement->let_statement, symbol_table);
    type_check_expression(statement->condition, symbol_table);
    type_check_statement(statement->update, symbol_table);


    if(statement->min->type_info->type != TypeInfoType::INT || statement->max->type_info->type != TypeInfoType::INT) {
        panic("Need to use int types in for loop");
    }
    auto min_type_info = static_cast<IntTypeInfo*>(statement->min->type_info);

    auto table_copy = *symbol_table;
    // TODO line info here
    table_copy.add_identifier(Token(TokenType::TOKEN_IDENTIFIER, "i", 0, 0), min_type_info);
    type_check_scope_statement(statement->body, &table_copy, false);*/
}

void TypeChecker::
type_check_if_statement(IfStatement* statement, SymbolTable* symbol_table) {
    type_check_expression(statement->expression, symbol_table);
    if(!type_match(statement->expression->type_info, symbol_table->get_type("bool"))) {
        panic("If statement must be passed a boolean");
    }
    type_check_scope_statement(statement->body, symbol_table);
}

void TypeChecker::
type_check_struct_statement(StructStatement* statement, SymbolTable* symbol_table, bool first_pass) {
	
	auto members_type_info = std::vector<std::tuple<std::string, TypeInfo*>>();
	for (auto& [member, expr] : statement->members) {
		type_check_type_expression(expr, symbol_table);
		members_type_info.emplace_back(member.string, expr->type_info);
	}

    if(first_pass) {
        symbol_table->type_table[statement->identifier.string] = new StructTypeInfo{TypeInfoType::STRUCT, members_type_info};
    } else {
        symbol_table->add_identifier(statement->identifier, new StructTypeInfo{TypeInfoType::STRUCT, members_type_info});
    }
}
void TypeChecker::
type_check_assigment_statement(AssigmentStatement* statement, SymbolTable* symbol_table) {

	if (!symbol_table->identifier_table.contains(statement->identifier.string)) {
		panic("Trying to assign un-declared identifier");
	}

	type_check_expression(statement->assigned_to->expression, symbol_table);

	if (!type_match(symbol_table->identifier_table[statement->identifier.string], statement->assigned_to->expression->type_info)) {
		panic("Type mismatch, trying to assign a identifier to an expression of different return_type");
	}
}

void TypeChecker::
type_check_expression_statement(ExpressionStatement* statement, SymbolTable* symbol_table) {
	type_check_expression(statement->expression, symbol_table);
}

void TypeChecker::
type_check_expression(Expression* expression, SymbolTable* symbol_table) {
    switch (expression->type) {
        case ExpressionType::EXPRESSION_STRING_LITERAL:
            return type_check_string_literal_expression(dynamic_cast<StringLiteralExpression *>(expression),
                                                        symbol_table); break;
        case ExpressionType::EXPRESSION_INT_LITERAL:
            return type_check_int_literal_expression(dynamic_cast<IntLiteralExpression *>(expression), symbol_table); break;
        case ExpressionType::EXPRESSION_BOOL_LITERAL:
            return type_check_bool_literal_expression(dynamic_cast<BoolLiteralExpression*>(expression), symbol_table); break;
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
        case ExpressionType::EXPRESSION_GROUP:
            return type_check_group_expression(dynamic_cast<GroupExpression*>(expression), symbol_table);
        default:
            panic("Not implemented");
    }
}
void TypeChecker::
type_check_binary_expression(BinaryExpression* expression, SymbolTable* symbol_table) {
	type_check_expression(expression->left, symbol_table);
	type_check_expression(expression->right, symbol_table);

	if (!type_match(expression->left->type_info, expression->right->type_info)) {
		panic("Type mismatch in binary expression");
	}

    TypeInfo* info = nullptr;
    // logical ops - make return_type bool
    if(expression->op.type == TOKEN_AND || expression->op.type == TOKEN_OR) {
        if (expression->left->type_info->type != TypeInfoType::BOOL && expression->right->type_info->type != TypeInfoType::BOOL) {
            panic("Cannot use logical operators on non bool return_type");
        }
        info = symbol_table->get_type("bool");
    }       // math ops - make return_type value types (int, float ...)
    else if(expression->op.type == TOKEN_PLUS || expression->op.type == TOKEN_STAR) {
        if (expression->left->type_info->type != TypeInfoType::INT) {
            panic("Cannot use arithmatic operator on non number return_type");
        }
        info = expression->left->type_info;
    }  // must be using == or != which resolve to bool
    else {
        info = symbol_table->get_type("bool");
    }

    expression->type_info = info;
}
void TypeChecker::
type_check_string_literal_expression(StringLiteralExpression* expression, SymbolTable* symbol_table) {
	expression->type_info = new PointerTypeInfo{TypeInfoType::POINTER, symbol_table->builtin_type_table["char"]};
}

void TypeChecker::
type_check_int_literal_expression(IntLiteralExpression* expression, SymbolTable* symbol_table) {
    expression->type_info = new IntTypeInfo{TypeInfoType::INT, false, 64};
}

void TypeChecker::
type_check_bool_literal_expression(BoolLiteralExpression* expression, SymbolTable* symbol_table) {
    expression->type_info = new BoolTypeInfo{TypeInfoType::BOOL};
}

void TypeChecker::
type_check_unary_expression(UnaryExpression* expression, SymbolTable* symbol_table) {
    type_check_expression(expression->expression, symbol_table);

	TypeInfo* type_info;
	if (expression->op.type == TOKEN_AT) {
		type_info = new PointerTypeInfo{TypeInfoType::POINTER, expression->expression->type_info };
	}
	else if(expression->op.type == TOKEN_STAR) {
		if (expression->expression->type_info->type != TypeInfoType::POINTER) {
			panic("Cannot dereference non-pointer value");
		}

		auto ptr_type = static_cast<PointerTypeInfo*>(expression->expression->type_info);
		type_info = ptr_type->to;
	}
	else {
		panic("Unrecognised unary operator");
	}

    expression->type_info = type_info;
}
void TypeChecker::
type_check_call_expression(CallExpression* expression, SymbolTable* symbol_table) {
    type_check_expression(expression->identifier, symbol_table);
	auto type_of_callee = dynamic_cast<IdentifierExpression*>(expression->identifier);

    if(!type_of_callee) {
        panic("Can only call on identifier expressions");
    }

	auto arg_type_infos = std::vector<TypeInfo*>();
	for (auto arg : expression->args) {
		type_check_expression(arg, symbol_table);
        arg_type_infos.push_back(arg->type_info);
	}

	if (type_of_callee->type_info->type != TypeInfoType::FN) {
		panic("Can only call a function");
	}

	auto fn_type_info = static_cast<FnTypeInfo*>(type_of_callee->type_info);
	if (fn_type_info->args.size() != arg_type_infos.size()) {
		panic("Incorrect numbers of args passed to function call");
	}

	for (s32 i = 0; i < fn_type_info->args.size(); i++) {
		if (!type_match(fn_type_info->args.at(i), arg_type_infos.at(i))) {
			panic("Type mismatch at function call");
		}
	}

    expression->type_info = fn_type_info->return_type;
}
void TypeChecker::
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

    expression->type_info = type_info;
}

void TypeChecker::
type_check_get_expression(GetExpression* expression, SymbolTable* symbol_table) {
	type_check_expression(expression->lhs, symbol_table);
	if (expression->lhs->type_info->type != TypeInfoType::STRUCT) {
		panic("Cannot derive member from non struct return_type");
	}

	TypeInfo* member_type_info = nullptr;
	auto struct_type_info = static_cast<StructTypeInfo*>(expression->lhs->type_info);
	for (auto [identifier, member_type] : struct_type_info->members) {
		if (identifier == expression->member.string) {
			member_type_info = member_type;
		}
	}

	if (member_type_info == nullptr) {
		panic("Cannot find member in struct");
	}

    expression->type_info = member_type_info;
}


void TypeChecker::
type_check_new_expression(NewExpression* expression, SymbolTable* symbol_table) {
	
	// check its a return_type
	if (!symbol_table->type_table.contains(expression->identifier.string)) {
		panic("Unrecognised return_type in new statement");
	}

	// check it's a struct
	if (symbol_table->type_table[expression->identifier.string]->type != TypeInfoType::STRUCT) {
		panic("Cannot use new on non struct identifier");
	}

	auto struct_type_info = static_cast<StructTypeInfo*>(symbol_table->type_table[expression->identifier.string]);

	// collect members from new constructor
	auto members_type_info = std::vector<TypeInfo*>();
	for (auto expr : expression->expressions) {
		type_check_expression(expr, symbol_table);
		members_type_info.push_back(expr->type_info);
	}

	// check counts
	if (struct_type_info->members.size() != members_type_info.size()) {
		panic("Incorrect number of arguments in new constructor");
	}

	// check types
	for (s32 i = 0; i < members_type_info.size(); i++) {
		auto [member, type] = struct_type_info->members.at(i);
		if (!type_match(members_type_info.at(i), type)) {
			panic("Incorect arguments to new constructor");
		}
	}

    expression->type_info = struct_type_info;
}

void TypeChecker::
type_check_group_expression(GroupExpression* expression, SymbolTable* symbol_table) {
    type_check_expression(expression->expression, symbol_table);
    expression->type_info = expression->expression->type_info;
}

void TypeChecker::
type_check_type_expression(TypeExpression* type_expression, SymbolTable* symbol_table) {
    switch (type_expression->type) {
        case TypeExpressionType::TYPE_IDENTIFIER:
            type_check_identifier_type_expression(dynamic_cast<IdentifierTypeExpression *>(type_expression), symbol_table); break;
        case TypeExpressionType::TYPE_POINTER:
            type_check_pointer_type_expression(dynamic_cast<PointerTypeExpression *>(type_expression), symbol_table); break;
        default:
            panic("Not implemented for return_type checker");
    }
}

void TypeChecker::
type_check_identifier_type_expression(IdentifierTypeExpression* type_expression, SymbolTable* symbol_table) {
    auto type = symbol_table->get_type(&type_expression->identifier);
    if(type) {
        type_expression->type_info = type;
        return;
    }

    panic("Unrecognised return_type in return_type expression: " + type_expression->identifier.string);
}

void TypeChecker::
type_check_pointer_type_expression(PointerTypeExpression* type_expression, SymbolTable* symbol_table) {
    type_check_type_expression(type_expression->pointer_of, symbol_table);
    type_expression->type_info = new PointerTypeInfo{TypeInfoType::POINTER, type_expression->pointer_of->type_info};
}

bool type_match(TypeInfo* a, TypeInfo* b) {

    if(a->type == TypeInfoType::ANY && b->type == TypeInfoType::ANY) {
        panic("cannot match types that are both any, will be fixed xx");
    }

    // TODO: any to any is actually ambigious so this is a bug
    if (a->type != b->type) {
        if(a->type != TypeInfoType::ANY && b->type != TypeInfoType::ANY) {
            return false;
        }
    }

    if(a->type == TypeInfoType::ANY) return true;
    if(b->type == TypeInfoType::ANY) return true;

	if (a->type == TypeInfoType::VOID || a->type == TypeInfoType::BOOL || a->type == TypeInfoType::CHAR) { // values don't matter
		return true;
	}
	else if (a->type == TypeInfoType::INT) {
		auto int_a = static_cast<IntTypeInfo*>(a);
		auto int_b = static_cast<IntTypeInfo*>(b);

		if (int_a->is_signed == int_b->is_signed && int_a->size == int_b->size)
			return true;

		return false;
	}
	else if (a->type == TypeInfoType::FN) {
		auto fn_a = static_cast<FnTypeInfo*>(a);
		auto fn_b = static_cast<FnTypeInfo*>(b);

		if (type_match(fn_a->return_type, fn_b->return_type)) {
			for (s32 i = 0; i < fn_a->args.size(); i++) {
				if(type_match(fn_a->args.at(i), fn_b->args.at(i))) {}
				else { return false;  }
			}

			return true;
		}

		return false;
	}
	else if (a->type == TypeInfoType::STRUCT) {
		auto struct_a = static_cast<StructTypeInfo*>(a);
		auto struct_b = static_cast<StructTypeInfo*>(b);

		for (s32 i = 0; i < struct_a->members.size(); i++) {
			auto [a_member, a_type] = struct_a->members.at(i);
			auto [b_member, b_type] = struct_b->members.at(i);
			if (!type_match(a_type, b_type)) {
				return false;
			}
		}

		return true;
	}
	else if (a->type == TypeInfoType::POINTER) {
		auto ptr_a = static_cast<PointerTypeInfo*>(a);
		auto ptr_b = static_cast<PointerTypeInfo*>(b);

		if (ptr_a->to->type == TypeInfoType::VOID) return true; // void^ can be equal to T^, not other way around
		return ptr_a->to->type == ptr_b->to->type;
	}

    panic("Cannot return_type check this return_type info");
    return false;
}