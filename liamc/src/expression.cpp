#include "expression.h"

std::ostream &Expression::format(std::ostream &os) const {
    os << "()";
    return os;
}

IsExpression::IsExpression(Expression *expression, TypeExpression *type_expression, Token identifier) {
    this->expression      = expression;
    this->type_expression = type_expression;
    this->identifier      = identifier;
    this->type            = ExpressionType::EXPRESSION_IS;
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
}

IntLiteralExpression::IntLiteralExpression(const Token token) {
    this->token = token;
    this->type  = ExpressionType::EXPRESSION_INT_LITERAL;
}

StringLiteralExpression::StringLiteralExpression(const Token token) {
    this->token = token;
    this->type  = ExpressionType::EXPRESSION_STRING_LITERAL;
    this->span = Span {
        .line = token.line,
        .start = token.character_start,
        .end = token.character_end
    };
}

BoolLiteralExpression::BoolLiteralExpression(Token value) {
    this->value = value;
    this->type  = ExpressionType::EXPRESSION_BOOL_LITERAL;
}

IdentifierExpression::IdentifierExpression(const Token identifier) {
    this->identifier = identifier;
    this->type       = ExpressionType::EXPRESSION_IDENTIFIER;
}

CallExpression::CallExpression(
    Expression *identifier, std::vector<Expression *> args, std::vector<TypeExpression *> generics
) {
    this->identifier = identifier;
    this->args       = args;
    this->generics   = generics;
    this->type       = ExpressionType::EXPRESSION_CALL;
}

GetExpression::GetExpression(Expression *expression, Token member) {
    this->lhs    = expression;
    this->member = member;
    this->type   = ExpressionType::EXPRESSION_GET;
}

NewExpression::NewExpression(
    Token identifier, std::vector<TypeExpression *> generics, std::vector<Expression *> expressions
) {
    this->identifier  = identifier;
    this->generics    = generics;
    this->expressions = expressions;
    this->type        = ExpressionType::EXPRESSION_NEW;
}

GroupExpression::GroupExpression(Expression *expression) {
    this->expression = expression;
    this->type       = ExpressionType::EXPRESSION_GROUP;
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
    this->span = Span {
        .line = identifier.line,
        .start = identifier.character_start,
        .end = identifier.character_end
    };
}

UnaryTypeExpression::UnaryTypeExpression(Token op, TypeExpression *type_expression) {
    this->op              = op;
    this->type_expression = type_expression;
    this->type            = TypeExpressionType::TYPE_UNARY;
}

UnionTypeExpression::UnionTypeExpression(std::vector<TypeExpression *> type_expressions) {
    this->type_expressions = type_expressions;
    this->type             = TypeExpressionType::TYPE_UNION;
}

SpecifiedGenericsTypeExpression::SpecifiedGenericsTypeExpression(
    IdentifierTypeExpression *struct_type, std::vector<TypeExpression *> generics
) {
    this->struct_type = struct_type;
    this->generics    = generics;
    this->type        = TypeExpressionType::TYPE_SPECIFIED_GENERICS;
}