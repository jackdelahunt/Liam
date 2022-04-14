#include "statement.h"
#include "expression.h"

std::ostream& Statement::format(std::ostream& os) const {
    os << "()";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Statement& statement)
{
    return statement.format(os);
}

LetStatement::LetStatement(Token identifier, Expression* expression) {
    this->identifier = identifier;
    this->expression = expression;
}

std::ostream& LetStatement::format(std::ostream& os) const {
    os << "(let " << identifier.string << " " << *expression << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const LetStatement& statement)
{
    return statement.format(os);
}

FnStatement::FnStatement(Token identifier, std::vector<Statement*> body) {
    this->identifier = identifier;
    this->body = body;
}

std::ostream& FnStatement::format(std::ostream& os) const {
    os << "(fn " << identifier.string;
    for (auto s_ptr : body) {
        os << *s_ptr;
    }
    os << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const FnStatement& statement)
{
    return statement.format(os);
}

InsertStatement::InsertStatement(Expression* byte_code) {
    this->byte_code = byte_code;
}

std::ostream& InsertStatement::format(std::ostream& os) const {
    os << "(insert " << *byte_code << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const InsertStatement& statement)
{
    return statement.format(os);
}