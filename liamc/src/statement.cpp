#include "statement.h"
#include "expression.h"
#include <tuple>

std::ostream& Statement::
format(std::ostream& os) const {
    os << "()";
    return os;
}

ExpressionStatement::
ExpressionStatement(Expression* expression) {
    this->expression = expression;
    this->statement_type = StatementType::STATEMENT_EXPRESSION;
}

LetStatement::
LetStatement(Token identifier, Expression* expression, TypeExpression* type) {
    this->identifier = identifier;
    this->rhs = expression;
    this->type= type;
    this->statement_type = StatementType::STATEMENT_LET;
}

FnStatement::
FnStatement(Token identifier, CSV params, TypeExpression* type, ScopeStatement* body) {
    this->identifier = identifier;
    this->return_type = type;
    this->params = params;
    this->body = body;
    this->statement_type = StatementType::STATEMENT_FN;
}

LoopStatement::
LoopStatement(Token identifier, ScopeStatement* body) {
    this->identifier = identifier;
    this->body = body;
    this->statement_type = StatementType::STATEMENT_LOOP;
}

StructStatement::
StructStatement(Token identifier, CSV members) {
    this->identifier = identifier;
    this->members = members;
    this->statement_type = StatementType::STATEMENT_STRUCT;
}

AssigmentStatement::
AssigmentStatement(Token identifier, ExpressionStatement* assigned_to) {
    this->identifier = identifier;
    this->assigned_to = assigned_to;
    this->statement_type = StatementType::STATEMENT_ASSIGNMENT;
}

ScopeStatement::
ScopeStatement(std::vector<Statement*> statements) {
    this->statements = statements;
    this->statement_type = StatementType::STATEMENT_SCOPE;
}

InsertStatement::
InsertStatement(Expression* byte_code) {
    this->byte_code = byte_code;
    this->statement_type = StatementType::STATEMENT_INSERT;
}

ImportStatement::
ImportStatement(Expression* file) {
    this->file = file;
    this->statement_type = StatementType::STATEMENT_IMPORT;
}

ForStatement::
ForStatement(LetStatement* let_statement, Expression* condition, Statement* update, ScopeStatement* body) {
    this->let_statement = let_statement;
    this->condition = condition;
    this->update = update;
    this->body = body;
    this->statement_type = StatementType::STATEMENT_FOR;
}

IfStatement::
IfStatement(Expression* expression, ScopeStatement* body) {
    this->expression = expression;
    this->body = body;
    this->statement_type = StatementType::STATEMENT_IF;
}

ReturnStatement::
ReturnStatement(Expression* expression) {
    this->expression = expression;
    this->statement_type = StatementType::STATEMENT_RETURN;
}

BreakStatement::
BreakStatement(Token identifier) {
    this->identifier= identifier;
    this->statement_type = StatementType::STATEMENT_BREAK;
}