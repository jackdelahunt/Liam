#pragma once

#include "lexer.h"
#include "liam.h"
#include "expression.h"

typedef std::vector<std::tuple<Token, Expression*>> CSV;

struct Statement {
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
    Expression* type;

    LetStatement(Token identifier, Expression* expression, Expression* type);
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

struct ReturnStatement: Statement {
    Expression* expression;

    ReturnStatement(Expression* expression);
    std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const ReturnStatement& statement);