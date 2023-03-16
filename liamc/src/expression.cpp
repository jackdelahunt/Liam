#include "expression.h"

std::ostream &Expression::format(std::ostream &os) const {
    os << "()";
    return os;
}

BinaryExpression::BinaryExpression(Expression *left, Token op, Expression *right) {
    this->left  = left;
    this->op    = op;
    this->right = right;
    this->type  = ExpressionType::EXPRESSION_BINARY;
}

UnaryExpression::UnaryExpression(Expression *expression, Token op) {
    this->expression = expression;
    this->op         = op;
    this->type       = ExpressionType::EXPRESSION_UNARY;
    this->span       = Span{.line = expression->span.line, .start = op.span.start, .end = expression->span.end};
}

SubscriptExpression::SubscriptExpression(Expression *lhs, Expression *expression) {
    this->expression = expression;
    this->lhs        = lhs;
    this->span       = lhs->span;
    this->type       = ExpressionType::EXPRESSION_SUBSCRIPT;
}

NumberLiteralExpression::NumberLiteralExpression(Token token) {
    this->token = token;
    this->type  = ExpressionType::EXPRESSION_NUMBER_LITERAL;
    this->span  = token.span;
}

StringLiteralExpression::StringLiteralExpression(Token token) {
    this->token = token;
    this->type  = ExpressionType::EXPRESSION_STRING_LITERAL;
    this->span  = Span{.line = token.span.line, .start = token.span.start, .end = token.span.end};
}

BoolLiteralExpression::BoolLiteralExpression(Token value) {
    this->value = value;
    this->type  = ExpressionType::EXPRESSION_BOOL_LITERAL;
    this->span  = Span{.line = value.span.line, .start = value.span.start, .end = value.span.end};
}

IdentifierExpression::IdentifierExpression(const Token identifier) {
    this->identifier = identifier;
    this->type       = ExpressionType::EXPRESSION_IDENTIFIER;
    this->span       = Span{.line = identifier.span.line, .start = identifier.span.start, .end = identifier.span.end};
}

CallExpression::CallExpression(
    Expression *identifier, std::vector<Expression *> args, std::vector<TypeExpression *> generics
) {
    this->callee   = identifier;
    this->args     = args;
    this->generics = generics;
    this->type     = ExpressionType::EXPRESSION_CALL;
    this->span     = identifier->span;
}

GetExpression::GetExpression(Expression *expression, Token member) {
    this->lhs    = expression;
    this->member = member;
    this->type   = ExpressionType::EXPRESSION_GET;
    this->span   = Span{expression->span.line, expression->span.start, member.span.end};
}

GroupExpression::GroupExpression(Expression *expression) {
    this->expression = expression;
    this->type       = ExpressionType::EXPRESSION_GROUP;
    this->span       = expression->span;
}

NullLiteralExpression::NullLiteralExpression(Token token) {
    this->type = ExpressionType::EXPRESSION_NULL_LITERAL;
    this->span = token.span;
}

ZeroLiteralExpression::ZeroLiteralExpression(Token token) {
    this->type = ExpressionType::EXPRESSION_ZERO_LITERAL;
    this->span = token.span;
}

FnExpression::FnExpression(
    std::vector<std::tuple<Token, TypeExpression *>> params, TypeExpression *return_type, ScopeStatement *body
) {
    this->params      = std::move(params);
    this->return_type = return_type;
    this->body        = body;
    this->type        = ExpressionType::EXPRESSION_FN;
    this->span        = return_type->span; // FIXME: use a better place to put the span for the fn expression
}

SliceLiteralExpression::SliceLiteralExpression(TypeExpression *type_expression, std::vector<Expression *> expressions) {
    this->type_expression = type_expression;
    this->expressions     = expressions;
    this->type            = ExpressionType::EXPRESSION_SLICE_LITERAL;
    this->span            = type_expression->span;
}

InstantiateExpression::InstantiateExpression(Expression *expression) {
    this->expression = expression;
    this->span       = expression->span;
    this->type       = ExpressionType::EXPRESSION_INSTANTIATION;
}

StructInstanceExpression::StructInstanceExpression(
    Token identifier, std::vector<TypeExpression *> generics,
    std::vector<std::tuple<Token, Expression *>> named_expressions
) {
    this->identifier        = identifier;
    this->generics          = generics;
    this->named_expressions = named_expressions;
    this->span              = identifier.span;
    this->type              = ExpressionType::EXPRESSION_STRUCT_INSTANCE;
}

EnumInstanceExpression::EnumInstanceExpression(Token lhs, Token member, std::vector<Expression *> arguments) {
    this->lhs          = lhs;
    this->member       = member;
    this->arguments    = std::move(arguments);
    this->member_index = -1; // yes I am setting a u64 to -1, lulul
    this->span         = lhs.span;
    this->type         = ExpressionType::EXPRESSION_ENUM_INSTANCE;
}

std::ostream &TypeExpression::format(std::ostream &os) const {
    os << "()";
    return os;
}

std::ostream &operator<<(std::ostream &os, const TypeExpression &expression) {
    return expression.format(os);
}

IdentifierTypeExpression::IdentifierTypeExpression(Token identifier) {
    this->identifier = identifier;
    this->type       = TypeExpressionType::TYPE_IDENTIFIER;
    this->span       = identifier.span;
}

UnaryTypeExpression::UnaryTypeExpression(UnaryType unary_type, TypeExpression *type_expression) {
    this->unary_type      = unary_type;
    this->type_expression = type_expression;
    this->type            = TypeExpressionType::TYPE_UNARY;
    this->span            = type_expression->span;
}

SpecifiedGenericsTypeExpression::SpecifiedGenericsTypeExpression(
    TypeExpression *struct_type, std::vector<TypeExpression *> generics
) {
    this->struct_type = struct_type;
    this->generics    = generics;
    this->span = Span{.line = struct_type->span.line, .start = struct_type->span.start, .end = struct_type->span.end};
    this->type = TypeExpressionType::TYPE_SPECIFIED_GENERICS;
}

FnTypeExpression::FnTypeExpression(std::vector<TypeExpression *> params, TypeExpression *return_type) {
    this->params      = std::move(params);
    this->return_type = return_type;
    this->span        = return_type->span; // FIXME: this should be a better span, return type is just used for now
    this->type        = TypeExpressionType::TYPE_FN;
}
