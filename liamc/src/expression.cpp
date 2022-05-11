#include "expression.h"

std::ostream& Expression::format(std::ostream& os) const {
    os << "()";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Expression& expression)
{
    return expression.format(os);
}

BinaryExpression::BinaryExpression(Expression* left, Token op, Expression* right) {
    this->left = left;
    this->op = op;
    this->right = right;
    this->type = ExpressionType::EXPRESSION_BINARY;
}

std::ostream& BinaryExpression::format(std::ostream& os) const {
    os << "(" << *left << " " << op.string << " " << *right << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const BinaryExpression& expression)
{
    return expression.format(os);
}

UnaryExpression::UnaryExpression(Expression* expression, Token op) {
    this->expression = expression;
    this->op = op;
    this->type = ExpressionType::EXPRESSION_UNARY;
}

std::ostream& UnaryExpression::format(std::ostream& os) const {
    os << "(" << op.string << " " << *expression<< ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const UnaryExpression& expression)
{
    return expression.format(os);
}

ArrayExpression::ArrayExpression(std::vector<Expression*> expressions) {
    this->expressions = std::move(expressions);
    this->type = ExpressionType::EXPRESSION_ARRAY;
}

std::ostream& ArrayExpression::format(std::ostream& os) const {
    os << "(" ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ArrayExpression& expression) {
    return expression.format(os);
}

IntLiteralExpression::IntLiteralExpression(const Token token) {
    this->token = token;
    this->type = ExpressionType::EXPRESSION_INT_LITERAL;
}

std::ostream& IntLiteralExpression::format(std::ostream& os) const {
    os << "(" << token.string << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const IntLiteralExpression& expression)
{
    return expression.format(os);
}

StringLiteralExpression::StringLiteralExpression(const Token token) {
    this->token = token;
    this->type = ExpressionType::EXPRESSION_STRING_LITERAL;
}

std::ostream& StringLiteralExpression::format(std::ostream& os) const {
    os << "(\"" << token.string << "\")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const StringLiteralExpression& expression)
{
    return expression.format(os);
}

IdentifierExpression::IdentifierExpression(const Token identifier) {
    this->identifier = identifier;
    this->type = ExpressionType::EXPRESSION_IDENTIFIER;
}

std::ostream& IdentifierExpression::format(std::ostream& os) const {
    os << "(" << identifier.string << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const IdentifierExpression& expression) {
    return expression.format(os);
}

CallExpression::CallExpression(Expression* identifier, std::vector<Expression*> args) {
    this->identifier = identifier;
    this->args = args;
    this->type = ExpressionType::EXPRESSION_CALL;
}

std::ostream& CallExpression::format(std::ostream& os) const {
    os << "(" << *identifier << "(";
    for (auto args : args) {
        os << *args << " ";
    }
    os << ")" << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const CallExpression& expression) {
    return expression.format(os);
}

GetExpression::GetExpression(Expression* expression, Token member) {
    this->expression = expression;
    this->member= member;
    this->type = ExpressionType::EXPRESSION_GET;
}

std::ostream& GetExpression::format(std::ostream& os) const {
    os << "(" << *expression << "." << member.string << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const GetExpression& expression) {
    return expression.format(os);
}

NewExpression::NewExpression(Token identifier, std::vector<Expression*> expressions) {
    this->identifier = identifier;
    this->expressions = expressions;
    this->type = ExpressionType::EXPRESSION_NEW;
}

std::ostream& NewExpression::format(std::ostream& os) const {
    os << "(" << identifier.string << "{})";
    return os;
}

std::ostream& operator<<(std::ostream& os, const NewExpression& expression) {
    return expression.format(os);
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