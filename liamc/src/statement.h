#pragma once

#include "expression.h"
#include "lexer.h"
#include "parser.h"

struct Expression;
struct Token;
struct File;

typedef std::vector<std::tuple<Token, TypeExpression *>> CSV;

enum class StatementType {
    STATEMENT_EXPRESSION,
    STATEMENT_LET,
    STATEMENT_SCOPE,
    STATEMENT_FN,
    STATEMENT_LOOP,
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
    Token identifier;
    std::vector<Token> generics;
    CSV params;
    TypeExpression *return_type;
    ScopeStatement *body;
    bool is_extern;

    FnStatement(
        File *file, Token identifier, std::vector<Token> generics, CSV params, TypeExpression *type,
        ScopeStatement *body, bool is_extern
    );
};

struct StructStatement : Statement {
    Token identifier;
    std::vector<Token> generics;
    CSV members;
    bool is_extern;

    StructStatement(File *file, Token identifier, std::vector<Token> generics, CSV members, bool is_extern);
};

struct AssigmentStatement : Statement {
    Expression *lhs;
    ExpressionStatement *assigned_to;

    AssigmentStatement(File *file, Expression *lhs, ExpressionStatement *assigned_to);
};

struct InsertStatement : Statement {
    Expression *byte_code;

    InsertStatement(File *file, Expression *byte_code);
};

struct ImportStatement : Statement {
    Expression *path;

    ImportStatement(File *file, Expression *path);
};
struct ForStatement : Statement {
    LetStatement *let_statement;
    Expression *condition;
    Statement *update;
    ScopeStatement *body;

    ForStatement(
        File *file, LetStatement *let_statement, Expression *condition, Statement *update, ScopeStatement *body
    );
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
    Token identifier;

    BreakStatement(File *file, Token identifier);
};

struct EnumStatement : Statement {
    Token identifier;
    std::vector<Token> instances;

    EnumStatement(Token identifier, std::vector<Token> instances);
};