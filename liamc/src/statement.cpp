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

ExpressionStatement::ExpressionStatement(Expression* expression) {
    this->expression = expression;
}

std::ostream& ExpressionStatement::format(std::ostream& os) const {
    os << "(" << *expression << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ExpressionStatement& statement)
{
    return statement.format(os);
}

LetStatement::LetStatement(Token identifier, Expression* expression, Token type) {
    this->identifier = identifier;
    this->expression = expression;
    this->type= type;
}

std::ostream& LetStatement::format(std::ostream& os) const {
    os << "(let " << identifier.string << " " << *expression << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const LetStatement& statement)
{
    return statement.format(os);
}

FnStatement::FnStatement(Token identifier, std::vector<std::tuple<Token, Token>> params, Token type, ScopeStatement* body) {
    this->identifier = identifier;
    this->type = type;
    this->params = params;
    this->body = body;
}

std::ostream& FnStatement::format(std::ostream& os) const {
    os << "(fn " << identifier.string << " (";
    for (auto& param: params) {
        os << param._Myfirst._Val << ",";
    }
    os << ")" << *body << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const FnStatement& statement)
{
    return statement.format(os);
}

LoopStatement::LoopStatement(Token identifier, ScopeStatement* body) {
    this->identifier = identifier;
    this->body = body;
}

std::ostream& LoopStatement::format(std::ostream& os) const {
    os << "( " << identifier.string << " (" << *body << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const LoopStatement& statement)
{
    return statement.format(os);
}

AssigmentStatement::AssigmentStatement(Token identifier, ExpressionStatement* assigned_to) {
    this->identifier = identifier;
    this->assigned_to = assigned_to;
}

std::ostream& AssigmentStatement::format(std::ostream& os) const {
    os << "( " << identifier.string << " =" << *assigned_to;
    return os;
}

std::ostream& operator<<(std::ostream& os, const AssigmentStatement& statement)
{
    return statement.format(os);
}

ScopeStatement::ScopeStatement(std::vector<Statement*> body) {
    this->body = body;
}

std::ostream& ScopeStatement::format(std::ostream& os) const {
    os << "(";
    for (auto stmt : body) {
        os << stmt;
    }
    os << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ScopeStatement& statement)
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

ReturnStatement::ReturnStatement(Expression* expression) {
    this->expression = expression;
}

std::ostream& ReturnStatement::format(std::ostream& os) const {
    os << "(return)";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ReturnStatement& statement)
{
    return statement.format(os);
}