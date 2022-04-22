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
}

std::ostream& UnaryExpression::format(std::ostream& os) const {
    os << "(" << op.string << " " << *expression<< ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const UnaryExpression& expression)
{
    return expression.format(os);
}

IntLiteralExpression::IntLiteralExpression(const Token token) {
    this->token = token;
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
}

std::ostream& NewExpression::format(std::ostream& os) const {
    os << "(" << identifier.string << "{})";
    return os;
}

std::ostream& operator<<(std::ostream& os, const NewExpression& expression) {
    return expression.format(os);
}