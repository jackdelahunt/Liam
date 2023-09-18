#pragma once

#include <vector>

#include "liam.h"
#include "token.h"

struct EnumMember;
struct Statement;
struct ExpressionStatement;
struct LetStatement;
struct ScopeStatement;
struct FnStatement;
struct StructStatement;
struct AssigmentStatement;
struct ImportStatement;
struct ForStatement;
struct IfStatement;
struct ElseStatement;
struct ReturnStatement;
struct BreakStatement;
struct EnumMember;
struct EnumStatement;
struct ContinueStatement;
struct Expression;
struct BinaryExpression;
struct UnaryExpression;
struct SubscriptExpression;
struct NumberLiteralExpression;
struct StringLiteralExpression;
struct BoolLiteralExpression;
struct IdentifierExpression;
struct CallExpression;
struct GetExpression;
struct GroupExpression;
struct NullLiteralExpression;
struct ZeroLiteralExpression;
struct FnExpression;
struct SliceLiteralExpression;
struct InstantiateExpression;
struct StructInstanceExpression;
struct EnumInstanceExpression;
struct TypeExpression;
struct IdentifierTypeExpression;
struct UnaryTypeExpression;
struct SpecifiedGenericsTypeExpression;
struct FnTypeExpression;

struct CompilationUnit;

struct TypeInfo;
struct AnyTypeInfo;
struct VoidTypeInfo;
struct NumberTypeInfo;
struct BoolTypeInfo;
struct PointerTypeInfo;
struct PointerSliceTypeInfo;
struct StrTypeInfo;
struct TypeTypeInfo;
struct StructTypeInfo;
struct StructInstanceTypeInfo;
struct FnTypeInfo;
struct FnExpressionTypeInfo;
struct GenericTypeInfo;
struct EnumTypeInfo;

typedef std::vector<std::tuple<Token, TypeExpression *>> CSV;

enum Tags {
    TAG_EXTERN  = 0b00000001,
    TAG_PRIVATE = 0b00000010
};

enum class StatementType {
    STATEMENT_EXPRESSION,
    STATEMENT_LET,
    STATEMENT_SCOPE,
    STATEMENT_FN,
    STATEMENT_STRUCT,
    STATEMENT_ASSIGNMENT,
    STATEMENT_INSERT,
    STATEMENT_IMPORT,
    STATEMENT_RETURN,
    STATEMENT_BREAK,
    STATEMENT_FOR,
    STATEMENT_IF,
    STATEMENT_ELSE,
    STATEMENT_ENUM,
    STATEMENT_CONTINUE,
    STATEMENT_MATCH,
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
    EXPRESSION_FN,
    EXPRESSION_SLICE_LITERAL,
    EXPRESSION_INSTANTIATION,
    EXPRESSION_STRUCT_INSTANCE,
    EXPRESSION_ENUM_INSTANCE,
};

enum class UnaryType {
    POINTER,
    POINTER_SLICE
};

enum class TypeExpressionType {
    TYPE_IDENTIFIER,
    TYPE_UNARY,
    TYPE_SPECIFIED_GENERICS,
    TYPE_FN,
};

enum class TypeInfoType {
    ANY,
    VOID,
    NUMBER,
    BOOLEAN,
    STRING,
    FN,
    FN_EXPRESSION,
    STRUCT,
    STRUCT_INSTANCE,
    POINTER,
    POINTER_SLICE,
    SLICE,
    GENERIC,
    ENUM,
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

struct PointerSliceTypeInfo : TypeInfo {
    TypeInfo *to;

    PointerSliceTypeInfo(TypeInfo *to);
};

struct StrTypeInfo : TypeInfo {
    StrTypeInfo();
};

struct TypeTypeInfo : TypeInfo {
    TypeTypeInfo();
};

struct StructTypeInfo : TypeInfo {
    u8 flag_mask;

    std::vector<std::tuple<std::string, FnTypeInfo *>> member_functions;
    std::vector<std::tuple<std::string, TypeInfo *>> members;
    u64 generic_count;

    StructTypeInfo(
        u8 flag_mask, std::vector<std::tuple<std::string, FnTypeInfo *>> memberFunctions,
        std::vector<std::tuple<std::string, TypeInfo *>> members, u64 genericCount
    );
};

struct StructInstanceTypeInfo : TypeInfo {
    StructTypeInfo *struct_type;
    std::vector<TypeInfo *> generic_types;

    StructInstanceTypeInfo(StructTypeInfo *structType, std::vector<TypeInfo *> genericTypes);
};

struct FnTypeInfo : TypeInfo {
    u8 flag_mask;

    StructTypeInfo *parent_type;
    TypeInfo *return_type;
    std::vector<TypeInfo *> generic_type_infos;
    std::vector<TypeInfo *> args;

    FnTypeInfo(
        u8 flag_mask, StructTypeInfo *parentType, TypeInfo *returnType, std::vector<TypeInfo *> genericTypeInfos,
        std::vector<TypeInfo *> args
    );
};

struct FnExpressionTypeInfo : TypeInfo {
    TypeInfo *return_type;
    std::vector<TypeInfo *> args;

    FnExpressionTypeInfo(TypeInfo *returnType, std::vector<TypeInfo *> args);
};

struct GenericTypeInfo : TypeInfo {
    u64 id;

    GenericTypeInfo(u64 id);
};

struct EnumTypeInfo : TypeInfo {
    u8 flag_mask;

    std::vector<EnumMember> members;

    EnumTypeInfo(std::vector<EnumMember> members, u8 flag_mask);
};

/*
    ======= PATTERN MATCHES ========
*/
struct EnumMemberPatternMatch {
    Token identifier;
    std::vector<Token> matched_members;
    i64 enum_member_index; // filled in by the type checker, this is used to index into to enum members in the enum type
                           // info

    EnumMemberPatternMatch(Token identifier, std::vector<Token> matched_members);
};

/*
    ======= STATEMENTS ========
*/
struct Statement {
    StatementType statement_type;
    CompilationUnit *file = NULL;
    virtual std::ostream &format(std::ostream &os) const;
};

struct ExpressionStatement : Statement {
    Expression *expression;

    ExpressionStatement(CompilationUnit *file, Expression *expression);
};

struct LetStatement : Statement {
    TokenIndex identifier;
    Expression *rhs;
    TypeExpression *type;

    LetStatement(CompilationUnit *file, TokenIndex identifier, Expression *expression, TypeExpression *type);
};

struct ScopeStatement : Statement {
    std::vector<Statement *> statements;

    ScopeStatement(CompilationUnit *file, std::vector<Statement *> statements);
};

struct FnStatement : Statement {
    TypeExpression *parent_type;
    TokenIndex identifier;
    std::vector<Token> generics;
    CSV params;
    TypeExpression *return_type;
    ScopeStatement *body;
    u8 flag_mask;

    FnStatement(
        CompilationUnit *file, TypeExpression *parent_type, TokenIndex identifier, std::vector<Token> generics, CSV params,
        TypeExpression *type, ScopeStatement *body, u8 flag_mask
    );
};

struct StructStatement : Statement {
    Token identifier;
    std::vector<Token> generics;
    CSV members;
    u8 flag_mask;

    StructStatement(CompilationUnit *file, Token identifier, std::vector<Token> generics, CSV members, u8 flag_mask);
};

struct AssigmentStatement : Statement {
    Expression *lhs;
    ExpressionStatement *assigned_to;

    AssigmentStatement(CompilationUnit *file, Expression *lhs, ExpressionStatement *assigned_to);
};

struct ImportStatement : Statement {
    StringLiteralExpression *path;

    ImportStatement(CompilationUnit *file, StringLiteralExpression *path);
};

struct ForStatement : Statement {
    Statement *assign;
    Expression *condition;
    Statement *update;
    ScopeStatement *body;

    ForStatement(
        CompilationUnit *file, Statement *assign, Expression *condition, Statement *update, ScopeStatement *body
    );
};

struct IfStatement : Statement {
    Expression *expression;
    ScopeStatement *body;
    ElseStatement *else_statement;

    IfStatement(CompilationUnit *file, Expression *expression, ScopeStatement *body, ElseStatement *else_statement);
};

struct ElseStatement : Statement {
    IfStatement *if_statement;
    ScopeStatement *body;

    ElseStatement(IfStatement *if_statement, ScopeStatement *body);
};

struct ReturnStatement : Statement {
    Expression *expression;

    ReturnStatement(CompilationUnit *file, Expression *expression);
};

struct BreakStatement : Statement {
    BreakStatement(CompilationUnit *file);
};

struct EnumMember {
    Token identifier;
    std::vector<TypeExpression *> members;

    EnumMember(Token identifier, std::vector<TypeExpression *> members);
};

struct EnumStatement : Statement {
    Token identifier;
    std::vector<EnumMember> members;
    u8 flag_mask;

    EnumStatement(CompilationUnit *file, Token identifier, std::vector<EnumMember> members, u8 flag_mask);
};

struct MatchStatement : Statement {
    Expression *matching_expression;
    std::vector<EnumMemberPatternMatch> pattern_matches;
    std::vector<ScopeStatement *> pattern_match_arms;

    MatchStatement(
        Expression *matching_expression, std::vector<EnumMemberPatternMatch> pattern_matches,
        std::vector<ScopeStatement *> pattern_match_arms
    );
};

struct ContinueStatement : Statement {
    ContinueStatement(CompilationUnit *file);
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
    Token op;
    Expression *right;

    BinaryExpression(Expression *left, Token op, Expression *right);
};

struct UnaryExpression : Expression {
    Token op;
    Expression *expression;

    UnaryExpression(Expression *expression, Token op);
};

struct SubscriptExpression : Expression {
    Expression *lhs;
    Expression *expression;

    SubscriptExpression(Expression *lhs, Expression *expression);
};

struct NumberLiteralExpression : Expression {
    Token token;
    i64 number;

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
    Expression *callee;
    std::vector<Expression *> args;
    std::vector<TypeExpression *> generics;

    CallExpression(Expression *identifier, std::vector<Expression *> args, std::vector<TypeExpression *> generics);
};

struct GetExpression : Expression {
    Expression *lhs;
    Token member;

    GetExpression(Expression *expression, Token member);
};

struct GroupExpression : Expression {
    Expression *expression;

    GroupExpression(Expression *expression);
};

struct NullLiteralExpression : Expression {
    NullLiteralExpression(Token token);
};

struct ZeroLiteralExpression : Expression {
    ZeroLiteralExpression(Token token);
};

struct FnExpression : Expression {
    std::vector<std::tuple<Token, TypeExpression *>> params;
    TypeExpression *return_type;
    ScopeStatement *body;

    FnExpression(
        std::vector<std::tuple<Token, TypeExpression *>> params, TypeExpression *return_type, ScopeStatement *body
    );
};

struct SliceLiteralExpression : Expression {
    TypeExpression *type_expression;
    std::vector<Expression *> expressions;

    SliceLiteralExpression(TypeExpression *type_expression, std::vector<Expression *> expressions);
};

struct InstantiateExpression : Expression {
    Expression *expression;

    InstantiateExpression(Expression *expression);
};

struct StructInstanceExpression : Expression {
    Token identifier;
    std::vector<TypeExpression *> generics;
    std::vector<std::tuple<Token, Expression *>> named_expressions;

    StructInstanceExpression(
        Token identifier, std::vector<TypeExpression *> generics,
        std::vector<std::tuple<Token, Expression *>> named_expressions
    );
};

struct EnumInstanceExpression : Expression {
    Token lhs;
    Token member;
    std::vector<Expression *> arguments;
    u64 member_index; // populated at type checking time

    EnumInstanceExpression(Token lhs, Token member, std::vector<Expression *> arguments);
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
    Token identifier;

    IdentifierTypeExpression(Token identifier);
};

struct UnaryTypeExpression : TypeExpression {
    UnaryType unary_type;
    TypeExpression *type_expression;

    UnaryTypeExpression(UnaryType unary_type, TypeExpression *type_expression);
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
