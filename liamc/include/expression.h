#pragma once
#include "lexer.h"
#include "liam.h"

enum class ExpressionType {
    EXPRESSION_BINARY,
    EXPRESSION_UNARY,
    EXPRESSION_INT_LITERAL,
    EXPRESSION_STRING_LITERAL,
    EXPRESSION_BOOL_LITERAL,
    EXPRESSION_IDENTIFIER,
    EXPRESSION_CALL,
    EXPRESSION_GET,
    EXPRESSION_NEW,
    EXPRESSION_ARRAY,
    EXPRESSION_ARRAY_SUBSCRIPT,
};

struct Expression {
    ExpressionType type;
    virtual std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const Expression& expression);

struct BinaryExpression : Expression {
    Expression* left;
    Token op;
    Expression* right;

    BinaryExpression(Expression* left, Token op, Expression* right);
};

struct UnaryExpression : Expression {
    Token op;
    Expression* expression;

    UnaryExpression(Expression* expression, Token op);
};

struct ArrayExpression : Expression {
    std::vector<Expression*> expressions;

    ArrayExpression(std::vector<Expression*> expressions);
};

struct IntLiteralExpression : Expression {
    Token token;

    IntLiteralExpression(const Token token);
};

struct StringLiteralExpression : Expression {
    Token token;

    StringLiteralExpression(const Token token);
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
    Expression* identifier;
    std::vector<Expression*> args;

    CallExpression(Expression* identifier, std::vector<Expression*> args);
};

struct GetExpression : Expression {
    // this is an expression but it must be a identifier
    Expression* expression;
    Token member;

    GetExpression(Expression* expression, Token member);
};

struct NewExpression : Expression {
    // this is an expression but it must be a identifier
    Token identifier;
    std::vector<Expression*> expressions;

    NewExpression(Token identifier, std::vector<Expression*> expressions);
};

struct ArraySubscriptExpression : Expression {
    // this is an expression but it must be a identifier
    Expression* array;
    Expression* subscript;

    ArraySubscriptExpression(Expression* array, Expression* subscript);
};

enum class TypeExpressionType {
    TYPE_IDENTIFIER,
    TYPE_POINTER,
    TYPE_ARRAY,
};

struct TypeExpression {
    TypeExpressionType type;
    virtual std::ostream& format(std::ostream& os) const;
};

struct IdentifierTypeExpression : TypeExpression {
    Token identifier;

    IdentifierTypeExpression(Token identifier);
};

struct PointerTypeExpression : TypeExpression {
    TypeExpression* pointer_of;

    PointerTypeExpression(TypeExpression* pointer_of);
};

struct ArrayTypeExpression : TypeExpression {
    TypeExpression* array_of;

    ArrayTypeExpression(TypeExpression* array_of);
};