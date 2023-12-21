#pragma once

#include <vector>

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
struct ImportStatement;
struct PrintStatement;
struct AssertStatement;
struct WhileStatement;

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
struct StaticArrayExpression;
struct SubscriptExpression;
struct RangeExpression;

struct TypeExpression;
struct IdentifierTypeExpression;
struct UnaryTypeExpression;
struct StaticArrayTypeExpression;
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
struct StaticArrayTypeInfo;
struct SliceTypeInfo;
struct RangeTypeInfo;

typedef std::vector<std::tuple<TokenIndex, TypeExpression *>> CSV;

enum class StatementType {
    UNDEFINED = 0,
    EXPRESSION,
    LET,
    SCOPE,
    FN,
    STRUCT,
    ASSIGNMENT,
    RETURN,
    BREAK,
    FOR,
    IF,
    ELSE,
    CONTINUE,
    IMPORT,
    PRINT,
    ASSERT,
    WHILE
};

enum class ExpressionType {
    UNDEFINED = 0,
    BINARY,
    UNARY,
    SUBSCRIPT,
    NUMBER_LITERAL,
    STRING_LITERAL,
    BOOL_LITERAL,
    IDENTIFIER,
    CALL,
    GET,
    GROUP,
    NULL_LITERAL,
    ZERO_LITERAL,
    INSTANTIATION,
    STRUCT_INSTANCE,
    STATIC_ARRAY,
    RANGE
};

enum class ExpressionCategory {
    UNDEFINED = 0,
    LVALUE,
    RVALUE
};

enum class ForType {
    UNDEFINED = 0,
    RANGE,
    STATIC_ARRAY,
    SLICE
};

enum class UnaryType {
    POINTER_DEREFERENCE,
    NOT,
    POINTER,
    MINUS
};

enum class TypeExpressionType {
    TYPE_IDENTIFIER,
    TYPE_UNARY,
    TYPE_GET,
    TYPE_STATIC_ARRAY,
    TYPE_SLICE
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
    NAMESPACE,
    STATIC_ARRAY,
    SLICE,
    RANGE
};

enum class NumberType {
    UNSIGNED,
    SIGNED,
    FLOAT
};

// need the SIZE_ prefix becase compiler is a pain
enum class NumberSize {
    SIZE_8,
    SIZE_16,
    SIZE_32,
    SIZE_64
};

union NumberValue {
    u64 u;
    i64 i;
    f64 f;
};

struct TypeInfo {
    TypeInfoType type;
};

struct AnyTypeInfo : TypeInfo {};

struct VoidTypeInfo : TypeInfo {
    VoidTypeInfo();
};

struct NumberTypeInfo : TypeInfo {
    NumberSize size;
    NumberType number_type;

    NumberTypeInfo(NumberSize size, NumberType number_type);
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
    StructStatement                                 *defined_location;
    std::vector<std::tuple<std::string, TypeInfo *>> members;

    StructTypeInfo(StructStatement *defined_location, std::vector<std::tuple<std::string, TypeInfo *>> members);
};

struct FnTypeInfo : TypeInfo {
    TypeInfo               *return_type;
    std::vector<TypeInfo *> args;

    FnTypeInfo(TypeInfo *returnType, std::vector<TypeInfo *> args);
};

struct NamespaceTypeInfo : TypeInfo {
    u64 compilation_unit_index;

    NamespaceTypeInfo(u64 compilation_unit_index);
};

struct StaticArrayTypeInfo : TypeInfo {
    u64       size;
    TypeInfo *base_type;

    StaticArrayTypeInfo(u64 size, TypeInfo *base_type);
};

struct SliceTypeInfo : TypeInfo {
    TypeInfo *base_type;

    SliceTypeInfo(TypeInfo *base_type);
};

// right now we are not tracking the types that we are ranging with in the range type info
// maybe in the future that would be useful but right now there is no need.
// You can either check if the range valid when creating the range expression
// or store the type in the range type and then check it when you are ranging
// in the static array or slice where ever. I think the first option is better
// for now atleast. Maybe in the future if you have very exotic types you might
// you can range on that depend on how you are using them the second method might
// be better.
// right now only number types can be the expression used in the range expression
// $NUMBER..NUMBER
struct RangeTypeInfo : TypeInfo {
    RangeTypeInfo();
};

/*
    ======= STATEMENTS ========
*/
struct Statement {
    StatementType statement_type = StatementType::UNDEFINED;
};

struct ExpressionStatement : Statement {
    Expression *expression;

    ExpressionStatement(Expression *expression);
};

struct LetStatement : Statement {
    TokenIndex      identifier;
    Expression     *rhs;
    TypeExpression *type;

    LetStatement(TokenIndex identifier, Expression *expression, TypeExpression *type);
};

struct ScopeStatement : Statement {
    std::vector<Statement *> statements;

    ScopeStatement(std::vector<Statement *> statements);
};

struct FnStatement : Statement {
    CompilationUnit *compilation_unit;
    TokenIndex       identifier;
    CSV              params;
    TypeExpression  *return_type;
    ScopeStatement  *body;

    FnStatement(CompilationUnit *compilation_unit, TokenIndex identifier, CSV params, TypeExpression *type,
                ScopeStatement *body);
};

struct StructStatement : Statement {
    CompilationUnit *compilation_unit;
    TokenIndex       identifier;
    CSV              members;
    StructTypeInfo  *type_info;

    StructStatement(CompilationUnit *compilation_unit, TokenIndex identifier, CSV members, StructTypeInfo *type_info);
};

struct AssigmentStatement : Statement {
    Expression          *lhs;
    ExpressionStatement *assigned_to;

    AssigmentStatement(Expression *lhs, ExpressionStatement *assigned_to);
};

struct ForStatement : Statement {
    TokenIndex      value_identifier;
    Expression     *expression;
    ScopeStatement *body;
    ForType for_type; // this is set at type checking time, based on the type of the expression we are iterating over

    ForStatement(TokenIndex value_identifier, Expression *expression, ScopeStatement *body, ForType for_type);
};

struct IfStatement : Statement {
    Expression     *expression;
    ScopeStatement *body;
    ElseStatement  *else_statement;

    IfStatement(Expression *expression, ScopeStatement *body, ElseStatement *else_statement);
};

struct ElseStatement : Statement {
    IfStatement    *if_statement;
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
    TokenIndex         identifier;
    TokenIndex         string_literal;
    NamespaceTypeInfo *namespace_type_info;

    ImportStatement(TokenIndex identifier, TokenIndex string_literal, NamespaceTypeInfo *namespace_type_info);
};

struct PrintStatement : Statement {
    Expression *expression;

    PrintStatement(Expression *expression);
};

struct AssertStatement : Statement {
    Expression *expression;

    AssertStatement(Expression *expression);
};

struct WhileStatement : Statement {
    Expression     *expression;
    ScopeStatement *body;

    WhileStatement(Expression *expression, ScopeStatement *body);
};

/*
    ======= EXPRESSIONS ========
*/
struct Expression {
    Span                  span      = {};
    TypeInfo             *type_info = nullptr;
    ExpressionType        type      = ExpressionType::UNDEFINED;
    ExpressionCategory    category  = ExpressionCategory::UNDEFINED;
    virtual std::ostream &format(std::ostream &os) const;
};

std::ostream &operator<<(std::ostream &os, const Expression &expression);

struct BinaryExpression : Expression {
    Expression *left;
    TokenType   op;
    Expression *right;

    BinaryExpression(Expression *left, TokenType op, Expression *right);
};

struct UnaryExpression : Expression {
    UnaryType   unary_type;
    Expression *expression;

    UnaryExpression(UnaryType unary_type, Expression *expression);
};

struct NumberLiteralExpression : Expression {
    TokenIndex  token;
    NumberValue value; // filled in at type checking time

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
    Expression               *callee;
    std::vector<Expression *> args;

    CallExpression(Expression *identifier, std::vector<Expression *> args);
};

struct GetExpression : Expression {
    Expression *lhs;
    TokenIndex  member;

    GetExpression(Expression *expression, TokenIndex member);
};

struct GroupExpression : Expression {
    Expression *sub_expression;

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
    TypeExpression                                   *type_expression;
    std::vector<std::tuple<TokenIndex, Expression *>> named_expressions;

    StructInstanceExpression(TypeExpression                                   *type_expression,
                             std::vector<std::tuple<TokenIndex, Expression *>> named_expressions);
};

struct StaticArrayExpression : Expression {
    NumberLiteralExpression  *number;
    TypeExpression           *type_expression;
    std::vector<Expression *> expressions;

    StaticArrayExpression(NumberLiteralExpression *number, TypeExpression *type_expression,
                          std::vector<Expression *> expression);
};

struct SubscriptExpression : Expression {
    Expression *subscriptee;
    Expression *subscripter;

    SubscriptExpression(Expression *subscriptee, Expression *subscripter);
};

struct RangeExpression : Expression {
    Expression *start;
    Expression *end;

    RangeExpression(Expression *start, Expression *end);
};

/*
    ======= TYPE EXPRESSIONS ========
*/
struct TypeExpression {
    Span                  span      = {};
    TypeInfo             *type_info = nullptr;
    TypeExpressionType    type;
    virtual std::ostream &format(std::ostream &os) const;
};

struct IdentifierTypeExpression : TypeExpression {
    TokenIndex identifier;

    IdentifierTypeExpression(TokenIndex identifier, Span span);
};

struct UnaryTypeExpression : TypeExpression {
    UnaryType       unary_type;
    TypeExpression *type_expression;

    UnaryTypeExpression(UnaryType unary_type, TypeExpression *type_expression);
};

struct GetTypeExpression : TypeExpression {
    TypeExpression *type_expression;
    TokenIndex      identifier;

    GetTypeExpression(TypeExpression *type_expression, TokenIndex identifier);
};

struct StaticArrayTypeExpression : TypeExpression {
    NumberLiteralExpression *size;
    TypeExpression          *base_type;

    StaticArrayTypeExpression(NumberLiteralExpression *size, TypeExpression *base_type);
};

struct SliceTypeExpression : TypeExpression {
    TypeExpression *base_type;

    SliceTypeExpression(TypeExpression *base_type);
};
