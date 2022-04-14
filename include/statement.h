#pragma once

#include "lexer.h"
#include "liam.h"
#include "expression.h"

struct Statement {
    virtual std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const Statement& statement);

struct LetStatement : Statement {
    Token identifier;
    Expression* expression;

    LetStatement(Token identifier, Expression* expression);
    std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const LetStatement& statement);

struct FnStatement : Statement {
    Token identifier;
    std::vector<Statement*> body;

    FnStatement(Token identifier, std::vector<Statement*> body);
    std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const FnStatement& statement);

struct InsertStatement : Statement {
    Expression* byte_code;

    InsertStatement(Expression* byte_code);
    std::ostream& format(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const InsertStatement& statement);