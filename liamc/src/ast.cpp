#include "ast.h"
#include "compilation_unit.h"

#include <tuple>

VoidTypeInfo::VoidTypeInfo() {
    this->type = TypeInfoType::VOID;
}

NumberTypeInfo::NumberTypeInfo(size_t size, NumberType number_type) {
    this->size        = size;
    this->number_type = number_type;
    this->type        = TypeInfoType::NUMBER;
}

BoolTypeInfo::BoolTypeInfo() {
    this->type = TypeInfoType::BOOLEAN;
}

PointerTypeInfo::PointerTypeInfo(TypeInfo *to) {
    this->to   = to;
    this->type = TypeInfoType::POINTER;
}

StrTypeInfo::StrTypeInfo() {
    this->type = TypeInfoType::STRING;
}

TypeTypeInfo::TypeTypeInfo() {
}

StructTypeInfo::StructTypeInfo(
    StructStatement *defined_location, std::vector<std::tuple<std::string, TypeInfo *>> members
) {
    this->defined_location = defined_location;
    this->members          = members;
    this->type             = TypeInfoType::STRUCT;
}

FnTypeInfo::FnTypeInfo(TypeInfo *returnType, std::vector<TypeInfo *> args) {
    this->return_type = returnType;
    this->args        = args;
    this->type        = TypeInfoType::FN;
}

NamespaceTypeInfo::NamespaceTypeInfo(u64 compilation_unit_index) {
    this->compilation_unit_index = compilation_unit_index;
    this->type                   = TypeInfoType::NAMESPACE;
}

StaticArrayTypeInfo::StaticArrayTypeInfo(u64 size, TypeInfo *base_type) {
    this->base_type = base_type;
    this->size      = size;
    this->type      = TypeInfoType::STATIC_ARRAY;
}

SliceTypeInfo::SliceTypeInfo(TypeInfo *base_type) {
    this->base_type = base_type;
    this->type      = TypeInfoType::SLICE;
}

RangeTypeInfo::RangeTypeInfo() {
    this->type = TypeInfoType::RANGE;
}

ExpressionStatement::ExpressionStatement(Expression *expression) {
    this->expression     = expression;
    this->statement_type = StatementType::STATEMENT_EXPRESSION;
}

LetStatement::LetStatement(TokenIndex identifier, Expression *expression, TypeExpression *type) {
    this->identifier     = identifier;
    this->rhs            = expression;
    this->type           = type;
    this->statement_type = StatementType::STATEMENT_LET;
}

AssigmentStatement::AssigmentStatement(Expression *lhs, ExpressionStatement *assigned_to) {
    this->lhs            = lhs;
    this->assigned_to    = assigned_to;
    this->statement_type = StatementType::STATEMENT_ASSIGNMENT;
}

ScopeStatement::ScopeStatement(std::vector<Statement *> statements) {
    this->statements     = statements;
    this->statement_type = StatementType::STATEMENT_SCOPE;
}

FnStatement::FnStatement(TokenIndex identifier, CSV params, TypeExpression *type, ScopeStatement *body) {
    this->identifier     = identifier;
    this->return_type    = type;
    this->params         = params;
    this->body           = body;
    this->statement_type = StatementType::STATEMENT_FN;
}

StructStatement::StructStatement(
    CompilationUnit *compilation_unit, TokenIndex identifier, CSV members, StructTypeInfo *type_info
) {
    this->compilation_unit = compilation_unit;
    this->identifier       = identifier;
    this->members          = members;
    this->type_info        = type_info;
    this->statement_type   = StatementType::STATEMENT_STRUCT;
}

ForStatement::ForStatement(Statement *assign, Expression *condition, Statement *update, ScopeStatement *body) {
    this->assign         = assign;
    this->condition      = condition;
    this->update         = update;
    this->body           = body;
    this->statement_type = StatementType::STATEMENT_FOR;
}

IfStatement::IfStatement(Expression *expression, ScopeStatement *body, ElseStatement *else_statement) {
    this->expression     = expression;
    this->body           = body;
    this->else_statement = else_statement;
    this->statement_type = StatementType::STATEMENT_IF;
}

ElseStatement::ElseStatement(IfStatement *if_statement, ScopeStatement *body) {
    this->if_statement   = if_statement;
    this->body           = body;
    this->statement_type = StatementType::STATEMENT_ELSE;
}

ReturnStatement::ReturnStatement(Expression *expression) {
    this->expression     = expression;
    this->statement_type = StatementType::STATEMENT_RETURN;
}

BreakStatement::BreakStatement() {
    this->statement_type = StatementType::STATEMENT_BREAK;
}

ContinueStatement::ContinueStatement() {
    this->statement_type = StatementType::STATEMENT_CONTINUE;
}

ImportStatement::ImportStatement(
    TokenIndex identifier, TokenIndex string_literal, NamespaceTypeInfo *namespace_type_info
) {
    this->statement_type      = StatementType::STATEMENT_IMPORT;
    this->identifier          = identifier;
    this->string_literal      = string_literal;
    this->namespace_type_info = namespace_type_info;
}

PrintStatement::PrintStatement(Expression *expression) {
    this->statement_type = StatementType::STATEMENT_PRINT;
    this->expression     = expression;
}

std::ostream &Expression::format(std::ostream &os) const {
    os << "()";
    return os;
}

BinaryExpression::BinaryExpression(Expression *left, TokenType op, Expression *right) {
    this->left  = left;
    this->op    = op;
    this->right = right;
    this->type  = ExpressionType::EXPRESSION_BINARY;
}

UnaryExpression::UnaryExpression(Expression *expression, TokenType op) {
    this->expression = expression;
    this->op         = op;
    this->type       = ExpressionType::EXPRESSION_UNARY;
    this->span       = expression->span;
}

NumberLiteralExpression::NumberLiteralExpression(TokenIndex token, Span span) {
    this->token = token;
    this->type  = ExpressionType::EXPRESSION_NUMBER_LITERAL;
    this->span  = span;
}

StringLiteralExpression::StringLiteralExpression(TokenIndex token, Span span) {
    this->token = token;
    this->type  = ExpressionType::EXPRESSION_STRING_LITERAL;
    this->span  = span;
}

BoolLiteralExpression::BoolLiteralExpression(TokenIndex token, Span span) {
    this->token = token;
    this->type  = ExpressionType::EXPRESSION_BOOL_LITERAL;
    this->span  = span;
}

IdentifierExpression::IdentifierExpression(TokenIndex identifier, Span span) {
    this->identifier = identifier;
    this->type       = ExpressionType::EXPRESSION_IDENTIFIER;
    this->span       = span;
}

CallExpression::CallExpression(Expression *identifier, std::vector<Expression *> args) {
    this->callee = identifier;
    this->args   = args;
    this->type   = ExpressionType::EXPRESSION_CALL;
    this->span   = identifier->span;
}

GetExpression::GetExpression(Expression *expression, TokenIndex member) {
    this->lhs    = expression;
    this->member = member;
    this->type   = ExpressionType::EXPRESSION_GET;
    this->span   = expression->span;
}

GroupExpression::GroupExpression(Expression *expression) {
    this->expression = expression;
    this->type       = ExpressionType::EXPRESSION_GROUP;
    this->span       = expression->span;
}

NullLiteralExpression::NullLiteralExpression(TokenIndex token, Span span) {
    this->type  = ExpressionType::EXPRESSION_NULL_LITERAL;
    this->token = token;
    this->span  = span;
}

ZeroLiteralExpression::ZeroLiteralExpression(TokenIndex token, Span span) {
    this->type  = ExpressionType::EXPRESSION_ZERO_LITERAL;
    this->token = token;
    this->span  = span;
}

InstantiateExpression::InstantiateExpression(Expression *expression) {
    this->type       = ExpressionType::EXPRESSION_INSTANTIATION;
    this->expression = expression;
    this->span       = expression->span;
}

StructInstanceExpression::StructInstanceExpression(
    TypeExpression *type_expression, std::vector<std::tuple<TokenIndex, Expression *>> named_expressions
) {
    this->type              = ExpressionType::EXPRESSION_STRUCT_INSTANCE;
    this->type_expression   = type_expression;
    this->named_expressions = named_expressions;
    this->span              = type_expression->span;
}

StaticArrayExpression::StaticArrayExpression(
    NumberLiteralExpression *number, TypeExpression *type_expression, std::vector<Expression *> expressions
) {
    this->number          = number;
    this->type_expression = type_expression;
    this->expressions     = expressions;
    this->type            = ExpressionType::EXPRESSION_STATIC_ARRAY;
}

SubscriptExpression::SubscriptExpression(Expression *subscriptee, Expression *subscripter) {
    this->subscriptee = subscriptee;
    this->subscripter = subscripter;
    this->span        = subscripter->span;
    this->type        = ExpressionType::EXPRESSION_SUBSCRIPT;
}

RangeExpression::RangeExpression(Expression *start, Expression *end) {
    this->start = start;
    this->end   = end;
    this->span =
        Span{}; // TODO: each expression might be null here so we need to figure out what to do here, maybe use the : ??
    this->type = ExpressionType::EXPRESSION_RANGE;
}

std::ostream &TypeExpression::format(std::ostream &os) const {
    os << "()";
    return os;
}

std::ostream &operator<<(std::ostream &os, const TypeExpression &expression) {
    return expression.format(os);
}

IdentifierTypeExpression::IdentifierTypeExpression(TokenIndex identifier, Span span) {
    this->identifier = identifier;
    this->type       = TypeExpressionType::TYPE_IDENTIFIER;
    this->span       = span;
}

UnaryTypeExpression::UnaryTypeExpression(UnaryType unary_type, TypeExpression *type_expression) {
    this->unary_type      = unary_type;
    this->type_expression = type_expression;
    this->type            = TypeExpressionType::TYPE_UNARY;
    this->span            = type_expression->span;
}

GetTypeExpression::GetTypeExpression(TypeExpression *type_expression, TokenIndex identifier) {
    this->type_expression = type_expression;
    this->identifier      = identifier;
    this->type            = TypeExpressionType::TYPE_GET;
    this->span            = type_expression->span;
}

StaticArrayTypeExpression::StaticArrayTypeExpression(NumberLiteralExpression *size, TypeExpression *base_type) {
    this->size      = size;
    this->base_type = base_type;
    this->type      = TypeExpressionType::TYPE_STATIC_ARRAY;
    this->span      = base_type->span;
}

SliceTypeExpression::SliceTypeExpression(TypeExpression *base_type) {
    this->base_type = base_type;
    this->span      = base_type->span;
    this->type      = TypeExpressionType::TYPE_SLICE;
}
