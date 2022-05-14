#include "expression.h"

std::ostream& Expression::format(std::ostream& os) const {
    os << "()";
    return os;
}

BinaryExpression::BinaryExpression(Expression* left, Token op, Expression* right) {
    this->left = left;
    this->op = op;
    this->right = right;
    this->type = ExpressionType::EXPRESSION_BINARY;
}

UnaryExpression::UnaryExpression(Expression* expression, Token op) {
    this->expression = expression;
    this->op = op;
    this->type = ExpressionType::EXPRESSION_UNARY;
}

ArrayExpression::ArrayExpression(std::vector<Expression*> expressions) {
    this->expressions = std::move(expressions);
    this->type = ExpressionType::EXPRESSION_ARRAY;
}

IntLiteralExpression::IntLiteralExpression(const Token token) {
    this->token = token;
    this->type = ExpressionType::EXPRESSION_INT_LITERAL;
}

StringLiteralExpression::StringLiteralExpression(const Token token) {
    this->token = token;
    this->type = ExpressionType::EXPRESSION_STRING_LITERAL;
}

BoolLiteralExpression::BoolLiteralExpression(Token value) {
    this->value = value;
    this->type = ExpressionType::EXPRESSION_BOOL_LITERAL;
}

IdentifierExpression::IdentifierExpression(const Token identifier) {
    this->identifier = identifier;
    this->type = ExpressionType::EXPRESSION_IDENTIFIER;
}

CallExpression::CallExpression(Expression* identifier, std::vector<Expression*> args) {
    this->identifier = identifier;
    this->args = args;
    this->type = ExpressionType::EXPRESSION_CALL;
}

GetExpression::GetExpression(Expression* expression, Token member) {
    this->expression = expression;
    this->member= member;
    this->type = ExpressionType::EXPRESSION_GET;
}

NewExpression::NewExpression(Token identifier, std::vector<Expression*> expressions) {
    this->identifier = identifier;
    this->expressions = expressions;
    this->type = ExpressionType::EXPRESSION_NEW;
}

ArraySubscriptExpression::ArraySubscriptExpression(Expression* array, Expression* subscript) {
    this->array = array;
    this->subscript = subscript;
    this->type = ExpressionType::EXPRESSION_ARRAY_SUBSCRIPT;
}

std::ostream& TypeExpression::format(std::ostream& os) const {
    os << "()";
    return os;
}

std::ostream& operator<<(std::ostream& os, const TypeExpression& expression) {
    return expression.format(os);
}

IdentifierTypeExpression::IdentifierTypeExpression(Token identifier) {
    this->identifier = identifier;
    this->type = TypeExpressionType::TYPE_IDENTIFIER;
}
PointerTypeExpression::
PointerTypeExpression(TypeExpression* pointer_of) {
    this->pointer_of = pointer_of;
    this->type = TypeExpressionType::TYPE_POINTER;
}

ArrayTypeExpression::
ArrayTypeExpression(TypeExpression *array_of) {
    this->array_of = array_of;
    this->type = TypeExpressionType::TYPE_ARRAY;
}