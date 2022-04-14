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