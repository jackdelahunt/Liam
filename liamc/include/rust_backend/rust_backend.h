#pragma once
#include <string>
#include <map>
#include "parser.h"
#include "statement.h"
#include "expression.h"
#include "type_checker.h"

struct RustBackend {
	RustBackend() = default;
	std::string emit(TypedFile* file);

	std::string emit_statement(TypeCheckedStatement* statement);
	std::string emit_insert_statement(TypeCheckedInsertStatement* statement);
	std::string emit_return_statement(TypeCheckedReturnStatement* statement);
	std::string emit_break_statement(TypeCheckedBreakStatement* statement);
	std::string emit_let_statement(TypeCheckedLetStatement* statement);
	std::string emit_scope_statement(TypeCheckedScopeStatement* statement);
	std::string emit_fn_statement(TypeCheckedFnStatement* statement);
	std::string emit_loop_statement(TypeCheckedLoopStatement* statement);
	std::string emit_struct_statement(TypeCheckedStructStatement* statement);
	std::string emit_assigment_statement(TypeCheckedAssigmentStatement* statement);
    std::string emit_expression_statement(TypeCheckedExpressionStatement* statement);
    std::string emit_for_statement(TypeCheckedForStatement* statement);

    std::string emit_expression(TypeCheckedExpression* expression);
    std::string emit_cloneable_expression(TypeCheckedExpression* expression);
	std::string emit_binary_expression(TypeCheckedBinaryExpression* expression);
    std::string emit_string_literal_expression(TypeCheckedStringLiteralExpression* expression);
    std::string emit_bool_literal_expression(TypeCheckedBoolLiteralExpression* expression);
	std::string emit_int_literal_expression(TypeCheckedIntLiteralExpression* expression);
	std::string emit_unary_expression(TypeCheckedUnaryExpression* expression);
	std::string emit_call_expression(TypeCheckedCallExpression* expression);
	std::string emit_identifier_expression(TypeCheckedIdentifierExpression* expression);
	std::string emit_get_expression(TypeCheckedGetExpression* expression);
    std::string emit_new_expression(TypeCheckedNewExpression* expression);
    std::string emit_array_expression(TypeCheckedArrayExpression* expression);
    std::string emit_array_subscript_expression(TypeCheckedArraySubscriptExpression* expression);

    std::string emit_type_expression(TypeCheckedTypeExpression* type_expression);
    std::string emit_identifier_type_expression(TypeCheckedIdentifierTypeExpression* type_expression);
    std::string emit_pointer_type_expression(TypeCheckedPointerTypeExpression* type_expression);
    std::string emit_array_type_expression(TypeCheckedArrayTypeExpression* type_expression);
};