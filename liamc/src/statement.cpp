#include "statement.h"
#include "expression.h"
#include <tuple>

std::ostream& Statement::format(std::ostream& os) const {
    os << "()";
    return os;
}

ExpressionStatement::ExpressionStatement(Expression* expression) {
    this->expression = expression;
    this->statement_type = STATEMENT_EXPRESSION;
}

LetStatement::LetStatement(Token identifier, Expression* expression, TypeExpression* type) {
    this->identifier = identifier;
    this->expression = expression;
    this->type= type;
    this->statement_type = STATEMENT_LET;
}

FnStatement::FnStatement(Token identifier, CSV params, TypeExpression* type, ScopeStatement* body) {
    this->identifier = identifier;
    this->type = type;
    this->params = params;
    this->body = body;
    this->statement_type = STATEMENT_FN;
}

LoopStatement::LoopStatement(Token identifier, ScopeStatement* body) {
    this->identifier = identifier;
    this->body = body;
    this->statement_type = STATEMENT_LOOP;
}

StructStatement::StructStatement(Token identifier, CSV members) {
    this->identifier = identifier;
    this->members = members;
    this->statement_type = STATEMENT_STRUCT;
}

AssigmentStatement::AssigmentStatement(Token identifier, ExpressionStatement* assigned_to) {
    this->identifier = identifier;
    this->assigned_to = assigned_to;
    this->statement_type = STATEMENT_ASSIGNMENT;
}

ScopeStatement::ScopeStatement(std::vector<Statement*> body) {
    this->body = body;
    this->statement_type = STATEMENT_SCOPE;
}

InsertStatement::InsertStatement(Expression* byte_code) {
    this->byte_code = byte_code;
    this->statement_type = STATEMENT_INSERT;
}

ImportStatement::ImportStatement(Expression* file) {
    this->file = file;
    this->statement_type = STATEMENT_IMPORT;
}

ReturnStatement::ReturnStatement(Expression* expression) {
    this->expression = expression;
    this->statement_type = STATEMENT_RETURN;
}

BreakStatement::BreakStatement(Token identifier) {
    this->identifier= identifier;
    this->statement_type = STATEMENT_BREAK;
}