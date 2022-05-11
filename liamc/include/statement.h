#pragma once

#include "lexer.h"
#include "liam.h"
#include "expression.h"

struct Expression;
struct Token;

typedef std::vector<std::tuple<Token, TypeExpression*>> CSV;

enum StatementType {
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
    STATEMENT_BREAK
};

struct Statement {
    StatementType statement_type;
    virtual std::ostream& format(std::ostream& os) const;
};

struct ExpressionStatement : Statement {
    Expression* expression;

    ExpressionStatement(Expression* expression);
};

struct LetStatement : Statement {
    Token identifier;
    Expression* expression;
    TypeExpression* type;

    LetStatement(Token identifier, Expression* expression, TypeExpression* type);
};

struct ScopeStatement : Statement {
    std::vector<Statement*> body;

    ScopeStatement(std::vector<Statement*> body);
};

struct FnStatement : Statement {
    Token identifier;
    CSV params;
    TypeExpression* type;
    ScopeStatement* body;

    FnStatement(Token identifier, CSV params, TypeExpression* type, ScopeStatement* body);
};

struct LoopStatement : Statement {
    Token identifier;
    ScopeStatement* body;

    LoopStatement(Token identifier, ScopeStatement* body);
};

struct StructStatement : Statement {
    Token identifier;
    CSV members;

    StructStatement(Token identifier, CSV members);
};

struct AssigmentStatement : Statement {
    Token identifier;
    ExpressionStatement* assigned_to;

    AssigmentStatement(Token identifier, ExpressionStatement* assigned_to);
};

struct InsertStatement : Statement {
    Expression* byte_code;

    InsertStatement(Expression* byte_code);
};

struct ImportStatement : Statement {
    Expression* file;

    ImportStatement(Expression* file);
};

struct ReturnStatement: Statement {
    Expression* expression;

    ReturnStatement(Expression* expression);
};

struct BreakStatement : Statement {
    Token identifier;

    BreakStatement(Token identifier);
};