#pragma once
#include "lexer.h"
#include "type_info.h"
#include "statement.h"

struct TypeExpression;
struct ScopeStatement;

enum class ExpressionType {
    EXPRESSION_IS,
    EXPRESSION_BINARY,
    EXPRESSION_UNARY,
    EXPRESSION_NUMBER_LITERAL,
    EXPRESSION_STRING_LITERAL,
    EXPRESSION_BOOL_LITERAL,
    EXPRESSION_IDENTIFIER,
    EXPRESSION_CALL,
    EXPRESSION_GET,
    EXPRESSION_NEW,
    EXPRESSION_GROUP,
    EXPRESSION_NULL_LITERAL,
    EXPRESSION_PROPAGATE,
    EXPRESSION_ZERO_LITERAL,
    EXPRESSION_FN
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

struct NumberLiteralExpression : Expression {
    Token token;
    s64 number;

    NumberLiteralExpression(Token token);
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
    Expression *callee;
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
    std::vector<std::tuple<Token, Expression *>> named_expressions;

    NewExpression(
        Token identifier, std::vector<TypeExpression *> generics,
        std::vector<std::tuple<Token, Expression *>> named_expressions
    );
};

struct GroupExpression : Expression {
    Expression *expression;

    GroupExpression(Expression *expression);
};

struct NullLiteralExpression : Expression {
    NullLiteralExpression(Token token);
};

struct PropagateExpression : Expression {
    Expression *expression;
    TypeExpression *type_expression;
    TypeExpression *otherwise;

    PropagateExpression(Expression *expression, TypeExpression *type_expression, TypeExpression *otherwise);
};

struct ZeroLiteralExpression : Expression {
    ZeroLiteralExpression(Token token);
};

struct FnExpression : Expression {
    std::vector<std::tuple<Token, TypeExpression *>> params;
    TypeExpression *return_type;
    ScopeStatement *body;

    FnExpression(std::vector<std::tuple<Token, TypeExpression *>> params, TypeExpression *return_type, ScopeStatement *body);
};

enum class TypeExpressionType {
    TYPE_IDENTIFIER,
    TYPE_UNARY,
    TYPE_UNION,
    TYPE_SPECIFIED_GENERICS,
    TYPE_FN,
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
    TypeExpression *struct_type;
    std::vector<TypeExpression *> generics;

    SpecifiedGenericsTypeExpression(TypeExpression *struct_type, std::vector<TypeExpression *> generics);
};

struct FnTypeExpression : TypeExpression {
    std::vector<TypeExpression *> params;
    TypeExpression *return_type;

    FnTypeExpression(std::vector<TypeExpression *> params, TypeExpression *return_type);
};