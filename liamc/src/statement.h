#pragma once

#include "expression.h"
#include "lexer.h"
#include "parser.h"

struct Expression;
struct StringLiteralExpression;
struct Token;
struct File;
struct TypeExpression;

typedef std::vector<std::tuple<Token, TypeExpression *>> CSV;

enum Tags {
    TAG_EXTERN = 0b00000001
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
    STATEMENT_ALIAS,
    STATEMENT_MATCH,
};

struct EnumMember {
    Token identifier;
    std::vector<TypeExpression *> members;

    EnumMember(Token identifier, std::vector<TypeExpression *> members);
};

struct Statement {
    StatementType statement_type;
    File *file = NULL;
    virtual std::ostream &format(std::ostream &os) const;
};

struct ExpressionStatement : Statement {
    Expression *expression;

    ExpressionStatement(File *file, Expression *expression);
};

struct LetStatement : Statement {
    Token identifier;
    Expression *rhs;
    TypeExpression *type;

    LetStatement(File *file, Token identifier, Expression *expression, TypeExpression *type);
};

struct ScopeStatement : Statement {
    std::vector<Statement *> statements;

    ScopeStatement(File *file, std::vector<Statement *> statements);
};

struct FnStatement : Statement {
    TypeExpression *parent_type;
    Token identifier;
    std::vector<Token> generics;
    CSV params;
    TypeExpression *return_type;
    ScopeStatement *body;
    u8 flag_mask;

    FnStatement(
        File *file, TypeExpression *parent_type, Token identifier, std::vector<Token> generics, CSV params,
        TypeExpression *type, ScopeStatement *body, u8 flag_mask
    );
};

struct StructStatement : Statement {
    Token identifier;
    std::vector<Token> generics;
    CSV members;
    u8 flag_mask;

    StructStatement(File *file, Token identifier, std::vector<Token> generics, CSV members, u8 flag_mask);
};

struct AssigmentStatement : Statement {
    Expression *lhs;
    ExpressionStatement *assigned_to;

    AssigmentStatement(File *file, Expression *lhs, ExpressionStatement *assigned_to);
};

struct ImportStatement : Statement {
    StringLiteralExpression *path;

    ImportStatement(File *file, StringLiteralExpression *path);
};

struct ForStatement : Statement {
    Statement *assign;
    Expression *condition;
    Statement *update;
    ScopeStatement *body;

    ForStatement(File *file, Statement *assign, Expression *condition, Statement *update, ScopeStatement *body);
};

struct ElseStatement;

struct IfStatement : Statement {
    Expression *expression;
    ScopeStatement *body;
    ElseStatement *else_statement;

    IfStatement(File *file, Expression *expression, ScopeStatement *body, ElseStatement *else_statement);
};

struct ElseStatement : Statement {
    IfStatement *if_statement;
    ScopeStatement *body;

    ElseStatement(IfStatement *if_statement, ScopeStatement *body);
};

struct ReturnStatement : Statement {
    Expression *expression;

    ReturnStatement(File *file, Expression *expression);
};

struct BreakStatement : Statement {
    BreakStatement(File *file);
};

struct EnumStatement : Statement {
    Token identifier;
    std::vector<EnumMember> members;

    EnumStatement(File *file, Token identifier, std::vector<EnumMember> members);
};

struct ContinueStatement : Statement {
    ContinueStatement(File *file);
};

struct AliasStatement : Statement {
    Token identifier;
    TypeExpression *type_expression;

    AliasStatement(File *file, Token identifier, TypeExpression *type_expression);
};
