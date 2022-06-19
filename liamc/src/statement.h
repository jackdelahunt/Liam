#pragma once

#include "expression.h"
#include "lexer.h"

struct Expression;
struct Token;

typedef std::vector<std::tuple<Token, TypeExpression *>> CSV;

enum class StatementType
{
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
};

struct Statement
{
    StatementType statement_type;
    virtual std::ostream &format(std::ostream &os) const;
};

struct ExpressionStatement : Statement
{
    Expression *expression;

    ExpressionStatement(Expression *expression);
};

struct LetStatement : Statement
{
    Token identifier;
    Expression *rhs;
    TypeExpression *type;

    LetStatement(Token identifier, Expression *expression, TypeExpression *type);
};

struct ScopeStatement : Statement
{
    std::vector<Statement *> statements;

    ScopeStatement(std::vector<Statement *> statements);
};

struct FnStatement : Statement
{
    Token identifier;
    std::vector<Token> generics;
    CSV params;
    TypeExpression *return_type;
    ScopeStatement *body;
    bool is_extern;

    FnStatement(Token identifier, std::vector<Token> generics, CSV params, TypeExpression *type, ScopeStatement *body,
                bool is_extern);
};

struct LoopStatement : Statement
{
    Token identifier;
    ScopeStatement *body;

    LoopStatement(Token identifier, ScopeStatement *body);
};

struct StructStatement : Statement
{
    Token identifier;
    std::vector<Token> generics;
    CSV members;
    bool is_extern;

    StructStatement(Token identifier, std::vector<Token> generics, CSV members, bool is_extern);
};

struct AssigmentStatement : Statement
{
    Expression *lhs;
    ExpressionStatement *assigned_to;

    AssigmentStatement(Expression *lhs, ExpressionStatement *assigned_to);
};

struct InsertStatement : Statement
{
    Expression *byte_code;

    InsertStatement(Expression *byte_code);
};

struct ImportStatement : Statement
{
    Expression *file;

    ImportStatement(Expression *file);
};
struct ForStatement : Statement
{
    LetStatement *let_statement;
    Expression *condition;
    Statement *update;
    ScopeStatement *body;

    ForStatement(LetStatement *let_statement, Expression *condition, Statement *update, ScopeStatement *body);
};

struct IfStatement : Statement
{
    Expression *expression;
    ScopeStatement *body;

    IfStatement(Expression *expression, ScopeStatement *body);
};

struct ReturnStatement : Statement
{
    Expression *expression;

    ReturnStatement(Expression *expression);
};

struct BreakStatement : Statement
{
    Token identifier;

    BreakStatement(Token identifier);
};