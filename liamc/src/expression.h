#pragma once
#include "lexer.h"
#include "type_info.h"

struct TypeExpression;

enum class ExpressionType {
    EXPRESSION_IS,
    EXPRESSION_BINARY,
    EXPRESSION_UNARY,
    EXPRESSION_INT_LITERAL,
    EXPRESSION_STRING_LITERAL,
    EXPRESSION_BOOL_LITERAL,
    EXPRESSION_IDENTIFIER,
    EXPRESSION_CALL,
    EXPRESSION_GET,
    EXPRESSION_NEW,
    EXPRESSION_GROUP,
};

struct Expression {
    Span span           = {};
    TypeInfo *type_info = nullptr;
    ExpressionType type;
    virtual std::ostream &format(std::ostream &os) const;
};

std::ostream &operator<<(std::ostream &os, const Expression &expression);

struct IsExpression : Expression {
    Expression *expression;
    TypeExpression *type_expression;
    Token identifier;

    IsExpression(Expression *expression, TypeExpression *type_expression, Token identifier);
};

struct BinaryExpression : Expression {
    Expression *left;
    Token op;
    Expression *right;

    BinaryExpression(Expression *left, Token op, Expression *right);
};

struct UnaryExpression : Expression {
    Token op;
    Expression *expression;

    UnaryExpression(Expression *expression, Token op);
};

struct ArrayExpression : Expression {
    std::vector<Expression *> expressions;

    ArrayExpression(std::vector<Expression *> expressions);
};

struct IntLiteralExpression : Expression {
    Token token;

    IntLiteralExpression(Token token);
};

struct StringLiteralExpression : Expression {
    Token token;

    StringLiteralExpression(Token token);
};

struct BoolLiteralExpression : Expression {
    Token value;

    BoolLiteralExpression(Token value);
};

struct IdentifierExpression : Expression {
    Token identifier;

    IdentifierExpression(const Token identifier);
};

struct CallExpression : Expression {
    // this is an expression but it must be a identifier
    Expression *identifier;
    std::vector<Expression *> args;
    std::vector<TypeExpression *> generics;

    CallExpression(Expression *identifier, std::vector<Expression *> args, std::vector<TypeExpression *> generics);
};

struct GetExpression : Expression {
    // this is an expression but it must be a identifier
    Expression *lhs;
    Token member;

    GetExpression(Expression *expression, Token member);
};

struct NewExpression : Expression {
    Token identifier;
    std::vector<TypeExpression *> generics;
    std::vector<Expression *> expressions;

    NewExpression(Token identifier, std::vector<TypeExpression *> generics, std::vector<Expression *> expressions);
};

struct GroupExpression : Expression {
    Expression *expression;

    GroupExpression(Expression *expression);
};

enum class TypeExpressionType {
    TYPE_IDENTIFIER,
    TYPE_UNARY,
    TYPE_UNION,
    TYPE_SPECIFIED_GENERICS,
};

struct TypeExpression {
    Span span           = {};
    TypeInfo *type_info = nullptr;
    TypeExpressionType type;
    virtual std::ostream &format(std::ostream &os) const;
};

struct IdentifierTypeExpression : TypeExpression {
    Token identifier;

    IdentifierTypeExpression(Token identifier);
};

struct UnaryTypeExpression : TypeExpression {
    Token op;
    TypeExpression *type_expression;

    UnaryTypeExpression(Token op, TypeExpression *type_expression);
};

struct UnionTypeExpression : TypeExpression {
    std::vector<TypeExpression *> type_expressions;

    UnionTypeExpression(std::vector<TypeExpression *> type_expressions);
};

struct SpecifiedGenericsTypeExpression : TypeExpression {
    IdentifierTypeExpression *struct_type;
    std::vector<TypeExpression *> generics;

    SpecifiedGenericsTypeExpression(IdentifierTypeExpression *struct_type, std::vector<TypeExpression *> generics);
};