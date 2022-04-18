#pragma once
#include "lexer.h"
#include "liam.h"

struct Expression {
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

std::ostream& operator<<(std::ostream& os, const UnaryExpression& expression);

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

std::ostream& operator<<(std::ostream& os, const StringLiteralExpression& expression);

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