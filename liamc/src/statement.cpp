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
    this->statement_type = STATEMENT_EXPRESSION;
}

std::ostream& ExpressionStatement::format(std::ostream& os) const {
    os << "(" << *expression << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ExpressionStatement& statement)
{
    return statement.format(os);
}

LetStatement::LetStatement(Token identifier, Expression* expression, Expression* type) {
    this->identifier = identifier;
    this->expression = expression;
    this->type= type;
    this->statement_type = STATEMENT_LET;
}

std::ostream& LetStatement::format(std::ostream& os) const {
    os << "(let " << identifier.string << " " << *expression << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const LetStatement& statement)
{
    return statement.format(os);
}

FnStatement::FnStatement(Token identifier, CSV params, Expression* type, ScopeStatement* body) {
    this->identifier = identifier;
    this->type = type;
    this->params = params;
    this->body = body;
    this->statement_type = STATEMENT_FN;
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
    this->statement_type = STATEMENT_LOOP;
}

std::ostream& LoopStatement::format(std::ostream& os) const {
    os << "( " << identifier.string << " (" << *body << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const LoopStatement& statement)
{
    return statement.format(os);
}

StructStatement::StructStatement(Token identifier, CSV members) {
    this->identifier = identifier;
    this->members = members;
    this->statement_type = STATEMENT_STRUCT;
}

std::ostream& StructStatement::format(std::ostream& os) const {
    os << "( struct" << identifier.string + ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const StructStatement& statement)
{
    return statement.format(os);
}

AssigmentStatement::AssigmentStatement(Token identifier, ExpressionStatement* assigned_to) {
    this->identifier = identifier;
    this->assigned_to = assigned_to;
    this->statement_type = STATEMENT_ASSIGNMENT;
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
    this->statement_type = STATEMENT_SCOPE;
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
    this->statement_type = STATEMENT_INSERT;
}

std::ostream& InsertStatement::format(std::ostream& os) const {
    os << "(insert " << *byte_code << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const InsertStatement& statement)
{
    return statement.format(os);
}

ImportStatement::ImportStatement(Expression* file) {
    this->file = file;
    this->statement_type = STATEMENT_IMPORT;
}

std::ostream& ImportStatement::format(std::ostream& os) const {
    os << "(import " << *file << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ImportStatement& statement)
{
    return statement.format(os);
}

ReturnStatement::ReturnStatement(Expression* expression) {
    this->expression = expression;
    this->statement_type = STATEMENT_RETURN;
}

std::ostream& ReturnStatement::format(std::ostream& os) const {
    os << "(return)";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ReturnStatement& statement)
{
    return statement.format(os);
}

BreakStatement::BreakStatement(Token identifier) {
    this->identifier= identifier;
    this->statement_type = STATEMENT_BREAK;
}

std::ostream& BreakStatement::format(std::ostream& os) const {
    os << "(break)";
    return os;
}

std::ostream& operator<<(std::ostream& os, const BreakStatement& statement)
{
    return statement.format(os);
}