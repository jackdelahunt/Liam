#pragma once
#include "lexer.h"
#include "liam.h"

enum class ExpressionType {
    EXPRESSION_BINARY,
    EXPRESSION_UNARY,
    EXPRESSION_INT_LITERAL,
    EXPRESSION_STRING_LITERAL,
    EXPRESSION_IDENTIFIER,
    EXPRESSION_CALL,
    EXPRESSION_GET,
    EXPRESSION_NEW,
    EXPRESSION_ARRAY,
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
    std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const BinaryExpression& expression);

struct UnaryExpression : Expression {
    Token op;
    Expression* expression;

    UnaryExpression(Expression* expression, Token op);
    std::ostream& format(std::ostream& os) const;
};

struct ArrayExpression : Expression {
    std::vector<Expression*> expressions;

    ArrayExpression(std::vector<Expression*> expressions);
    std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const ArrayExpression& expression);

struct IntLiteralExpression : Expression {
    Token token;

    IntLiteralExpression(const Token token);
    std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const IntLiteralExpression& expression);


struct StringLiteralExpression : Expression {
    Token token;

    StringLiteralExpression(const Token token);
    std::ostream& format(std::ostream& os) const;
};

struct IdentifierExpression : Expression {
    Token identifier;

    IdentifierExpression(const Token identifier);
    std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const IdentifierExpression& expression);

struct CallExpression : Expression {
    // this is an expression but it must be a identifier
    Expression* identifier;
    std::vector<Expression*> args;

    CallExpression(Expression* identifier, std::vector<Expression*> args);
    std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const CallExpression& expression);

struct GetExpression : Expression {
    // this is an expression but it must be a identifier
    Expression* expression;
    Token member;

    GetExpression(Expression* expression, Token member);
    std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const GetExpression& expression);

struct NewExpression : Expression {
    // this is an expression but it must be a identifier
    Token identifier;
    std::vector<Expression*> expressions;

    NewExpression(Token identifier, std::vector<Expression*> expressions);
    std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const NewExpression& expression);

enum class TypeExpressionType {
    TYPE_IDENTIFIER,
    TYPE_POINTER,
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