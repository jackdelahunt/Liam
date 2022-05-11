#pragma once

#include "lexer.h"
#include "liam.h"
#include "expression.h"

struct Expression;
struct Token;

typedef std::vector<std::tuple<Token, Expression*>> CSV;

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

std::ostream& operator<<(std::ostream& os, const Statement& statement);

struct ExpressionStatement : Statement {
    Expression* expression;

    ExpressionStatement(Expression* expression);
    virtual std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const ExpressionStatement& statement);

struct LetStatement : Statement {
    Token identifier;
    Expression* expression;
    TypeExpression* type;

    LetStatement(Token identifier, Expression* expression, TypeExpression* type);
    std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const LetStatement& statement);

struct ScopeStatement : Statement {
    std::vector<Statement*> body;

    ScopeStatement(std::vector<Statement*> body);
    std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const ScopeStatement& statement);

struct FnStatement : Statement {
    Token identifier;
    CSV params;
    Expression* type;
    ScopeStatement* body;

    FnStatement(Token identifier, CSV params, Expression* type, ScopeStatement* body);
    std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const FnStatement& statement);

struct LoopStatement : Statement {
    Token identifier;
    ScopeStatement* body;

    LoopStatement(Token identifier, ScopeStatement* body);
    std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const LoopStatement& statement);

struct StructStatement : Statement {
    Token identifier;
    CSV members;

    StructStatement(Token identifier, CSV members);
    std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const LoopStatement& statement);

struct AssigmentStatement : Statement {
    Token identifier;
    ExpressionStatement* assigned_to;

    AssigmentStatement(Token identifier, ExpressionStatement* assigned_to);
    std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const AssigmentStatement& statement);

struct InsertStatement : Statement {
    Expression* byte_code;

    InsertStatement(Expression* byte_code);
    std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const InsertStatement& statement);

struct ImportStatement : Statement {
    Expression* file;

    ImportStatement(Expression* file);
    std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const ImportStatement& statement);

struct ReturnStatement: Statement {
    Expression* expression;

    ReturnStatement(Expression* expression);
    std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const ReturnStatement& statement);

struct BreakStatement : Statement {
    Token identifier;

    BreakStatement(Token identifier);
    std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const BreakStatement& statement);