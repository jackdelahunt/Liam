#pragma once

#include <vector>

#include "liam.h"
#include "token.h"

struct Statement;
struct ExpressionStatement;
struct LetStatement;
struct ScopeStatement;
struct FnStatement;
struct StructStatement;
struct AssigmentStatement;
struct ForStatement;
struct IfStatement;
struct ElseStatement;
struct ReturnStatement;
struct BreakStatement;
struct ContinueStatement;
struct Expression;
struct BinaryExpression;
struct UnaryExpression;
struct NumberLiteralExpression;
struct StringLiteralExpression;
struct BoolLiteralExpression;
struct IdentifierExpression;
struct CallExpression;
struct GetExpression;
struct GroupExpression;
struct NullLiteralExpression;
struct ZeroLiteralExpression;
struct InstantiateExpression;
struct StructInstanceExpression;
struct TypeExpression;
struct IdentifierTypeExpression;
struct UnaryTypeExpression;

struct CompilationUnit;

struct TypeInfo;
struct VoidTypeInfo;
struct NumberTypeInfo;
struct BoolTypeInfo;
struct PointerTypeInfo;
struct StrTypeInfo;
struct TypeTypeInfo;
struct StructTypeInfo;
struct FnTypeInfo;
struct NamespaceTypeInfo;

typedef std::vector<std::tuple<TokenIndex, TypeExpression *>> CSV;

enum class StatementType {
    STATEMENT_EXPRESSION,
    STATEMENT_LET,
    STATEMENT_SCOPE,
    STATEMENT_FN,
    STATEMENT_STRUCT,
    STATEMENT_ASSIGNMENT,
    STATEMENT_RETURN,
    STATEMENT_BREAK,
    STATEMENT_FOR,
    STATEMENT_IF,
    STATEMENT_ELSE,
    STATEMENT_CONTINUE,
    STATEMENT_IMPORT
};

enum class ExpressionType {
    EXPRESSION_BINARY,
    EXPRESSION_UNARY,
    EXPRESSION_SUBSCRIPT,
    EXPRESSION_NUMBER_LITERAL,
    EXPRESSION_STRING_LITERAL,
    EXPRESSION_BOOL_LITERAL,
    EXPRESSION_IDENTIFIER,
    EXPRESSION_CALL,
    EXPRESSION_GET,
    EXPRESSION_GROUP,
    EXPRESSION_NULL_LITERAL,
    EXPRESSION_ZERO_LITERAL,
    EXPRESSION_INSTANTIATION,
    EXPRESSION_STRUCT_INSTANCE,
};

enum class UnaryType {
    POINTER,
};

enum class TypeExpressionType {
    TYPE_IDENTIFIER,
    TYPE_UNARY,
    TYPE_GET
};

enum class TypeInfoType {
    ANY,
    VOID,
    NUMBER,
    BOOLEAN,
    STRING,
    FN,
    STRUCT,
    POINTER,
    NAMESPACE
};

enum class NumberType {
    UNSIGNED,
    SIGNED,
    FLOAT
};

struct TypeInfo {
    TypeInfoType type;
};

struct AnyTypeInfo : TypeInfo {};

struct VoidTypeInfo : TypeInfo {
    VoidTypeInfo();
};

struct NumberTypeInfo : TypeInfo {
    size_t size;
    NumberType number_type;

    NumberTypeInfo(size_t size, NumberType number_type);
};

struct BoolTypeInfo : TypeInfo {
    BoolTypeInfo();
};

struct PointerTypeInfo : TypeInfo {
    TypeInfo *to;

    PointerTypeInfo(TypeInfo *to);
};

struct StrTypeInfo : TypeInfo {
    StrTypeInfo();
};

struct TypeTypeInfo : TypeInfo {
    TypeTypeInfo();
};

struct StructTypeInfo : TypeInfo {
    std::vector<std::tuple<std::string, TypeInfo *>> members;

    StructTypeInfo(
        std::vector<std::tuple<std::string, TypeInfo *>> members
    );
};

struct FnTypeInfo : TypeInfo {
    TypeInfo *return_type;
    std::vector<TypeInfo *> args;

    FnTypeInfo(TypeInfo *returnType, std::vector<TypeInfo *> args);
};

struct NamespaceTypeInfo : TypeInfo {
    u64 compilation_unit_index;

    NamespaceTypeInfo(u64 compilation_unit_index);
};

/*
    ======= STATEMENTS ========
*/
struct Statement {
    StatementType statement_type;
};

struct ExpressionStatement : Statement {
    Expression *expression;

    ExpressionStatement(Expression *expression);
};

struct LetStatement : Statement {
    TokenIndex identifier;
    Expression *rhs;
    TypeExpression *type;

    LetStatement(TokenIndex identifier, Expression *expression, TypeExpression *type);
};

struct ScopeStatement : Statement {
    std::vector<Statement *> statements;

    ScopeStatement(std::vector<Statement *> statements);
};

struct FnStatement : Statement {
    TokenIndex identifier;
    CSV params;
    TypeExpression *return_type;
    ScopeStatement *body;

    FnStatement(TokenIndex identifier, CSV params, TypeExpression *type, ScopeStatement *body);
};

struct StructStatement : Statement {
    TokenIndex identifier;
    CSV members;

    StructStatement(TokenIndex identifier, CSV members);
};

struct AssigmentStatement : Statement {
    Expression *lhs;
    ExpressionStatement *assigned_to;

    AssigmentStatement(Expression *lhs, ExpressionStatement *assigned_to);
};

struct ForStatement : Statement {
    Statement *assign;
    Expression *condition;
    Statement *update;
    ScopeStatement *body;

    ForStatement(Statement *assign, Expression *condition, Statement *update, ScopeStatement *body);
};

struct IfStatement : Statement {
    Expression *expression;
    ScopeStatement *body;
    ElseStatement *else_statement;

    IfStatement(Expression *expression, ScopeStatement *body, ElseStatement *else_statement);
};

struct ElseStatement : Statement {
    IfStatement *if_statement;
    ScopeStatement *body;

    ElseStatement(IfStatement *if_statement, ScopeStatement *body);
};

struct ReturnStatement : Statement {
    Expression *expression;

    ReturnStatement(Expression *expression);
};

struct BreakStatement : Statement {
    BreakStatement();
};

struct ContinueStatement : Statement {
    ContinueStatement();
};

struct ImportStatement : Statement {
    TokenIndex identifier;
    TokenIndex string_literal;

    ImportStatement(TokenIndex identifier, TokenIndex string_literal);
};

/*
    ======= EXPRESSIONS ========
*/
struct Expression {
    Span span           = {};
    TypeInfo *type_info = nullptr;
    ExpressionType type;
    virtual std::ostream &format(std::ostream &os) const;
};

std::ostream &operator<<(std::ostream &os, const Expression &expression);

struct BinaryExpression : Expression {
    Expression *left;
    TokenType op;
    Expression *right;

    BinaryExpression(Expression *left, TokenType op, Expression *right);
};

struct UnaryExpression : Expression {
    TokenType op;
    Expression *expression;

    UnaryExpression(Expression *expression, TokenType op);
};

struct NumberLiteralExpression : Expression {
    TokenIndex token;
    i64 number;

    NumberLiteralExpression(TokenIndex token, Span span);
};

struct StringLiteralExpression : Expression {
    TokenIndex token;

    StringLiteralExpression(TokenIndex token, Span span);
};

struct BoolLiteralExpression : Expression {
    TokenIndex token;

    BoolLiteralExpression(TokenIndex token, Span span);
};

struct IdentifierExpression : Expression {
    TokenIndex identifier;

    IdentifierExpression(TokenIndex identifier, Span span);
};

struct CallExpression : Expression {
    Expression *callee;
    std::vector<Expression *> args;

    CallExpression(Expression *identifier, std::vector<Expression *> args);
};

struct GetExpression : Expression {
    Expression *lhs;
    TokenIndex member;

    GetExpression(Expression *expression, TokenIndex member);
};

struct GroupExpression : Expression {
    Expression *expression;

    GroupExpression(Expression *expression);
};

struct NullLiteralExpression : Expression {
    TokenIndex token;

    NullLiteralExpression(TokenIndex token, Span span);
};

struct ZeroLiteralExpression : Expression {
    TokenIndex token;

    ZeroLiteralExpression(TokenIndex token, Span span);
};

struct InstantiateExpression : Expression {
    Expression *expression;

    InstantiateExpression(Expression *expression);
};

struct StructInstanceExpression : Expression {
    TypeExpression *type_expression;
    std::vector<std::tuple<TokenIndex, Expression *>> named_expressions;

    StructInstanceExpression(
        TypeExpression *type_expression, std::vector<std::tuple<TokenIndex, Expression *>> named_expressions
    );
};

/*
    ======= TYPE EXPRESSIONS ========
*/
struct TypeExpression {
    Span span           = {};
    TypeInfo *type_info = nullptr;
    TypeExpressionType type;
    virtual std::ostream &format(std::ostream &os) const;
};

struct IdentifierTypeExpression : TypeExpression {
    TokenIndex identifier;

    IdentifierTypeExpression(TokenIndex identifier, Span span);
};

struct UnaryTypeExpression : TypeExpression {
    UnaryType unary_type;
    TypeExpression *type_expression;

    UnaryTypeExpression(UnaryType unary_type, TypeExpression *type_expression);
};

struct GetTypeExpression : TypeExpression {
    TypeExpression *type_expression;
    TokenIndex identifier;

    GetTypeExpression(TypeExpression *type_expression, TokenIndex identifier);
};
