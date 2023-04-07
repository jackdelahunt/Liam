#include "statement.h"

#include <tuple>

#include "expression.h"

EnumMember::EnumMember(Token identifier, std::vector<TypeExpression *> members) {
    this->identifier = identifier;
    this->members    = members;
}

std::ostream &Statement::format(std::ostream &os) const {
    os << "()";
    return os;
}

ExpressionStatement::ExpressionStatement(File *file, Expression *expression) {
    this->file           = file;
    this->expression     = expression;
    this->statement_type = StatementType::STATEMENT_EXPRESSION;
}

LetStatement::LetStatement(File *file, Token identifier, Expression *expression, TypeExpression *type) {
    this->identifier     = identifier;
    this->rhs            = expression;
    this->type           = type;
    this->statement_type = StatementType::STATEMENT_LET;
}

FnStatement::FnStatement(
    File *file, TypeExpression *parent_type, Token identifier, std::vector<Token> generics, CSV params,
    TypeExpression *type, ScopeStatement *body, u8 flag_mask
) {
    this->file           = file;
    this->parent_type    = parent_type;
    this->identifier     = identifier;
    this->generics       = generics;
    this->return_type    = type;
    this->params         = params;
    this->body           = body;
    this->flag_mask      = flag_mask;
    this->statement_type = StatementType::STATEMENT_FN;
}

StructStatement::StructStatement(File *file, Token identifier, std::vector<Token> generics, CSV members, u8 flag_mask) {
    this->file           = file;
    this->identifier     = identifier;
    this->generics       = generics;
    this->members        = members;
    this->flag_mask      = flag_mask;
    this->statement_type = StatementType::STATEMENT_STRUCT;
}

AssigmentStatement::AssigmentStatement(File *file, Expression *lhs, ExpressionStatement *assigned_to) {
    this->file           = file;
    this->lhs            = lhs;
    this->assigned_to    = assigned_to;
    this->statement_type = StatementType::STATEMENT_ASSIGNMENT;
}

ScopeStatement::ScopeStatement(File *file, std::vector<Statement *> statements) {
    this->file           = file;
    this->statements     = statements;
    this->statement_type = StatementType::STATEMENT_SCOPE;
}

ImportStatement::ImportStatement(File *file, StringLiteralExpression *path) {
    this->file           = file;
    this->path           = path;
    this->statement_type = StatementType::STATEMENT_IMPORT;
}

ForStatement::ForStatement(
    File *file, Statement *assign, Expression *condition, Statement *update, ScopeStatement *body
) {
    this->file           = file;
    this->assign         = assign;
    this->condition      = condition;
    this->update         = update;
    this->body           = body;
    this->statement_type = StatementType::STATEMENT_FOR;
}

IfStatement::IfStatement(File *file, Expression *expression, ScopeStatement *body, ElseStatement *else_statement) {
    this->file           = file;
    this->expression     = expression;
    this->body           = body;
    this->else_statement = else_statement;
    this->statement_type = StatementType::STATEMENT_IF;
}

ElseStatement::ElseStatement(IfStatement *if_statement, ScopeStatement *body) {
    this->if_statement   = if_statement;
    this->body           = body;
    this->statement_type = StatementType::STATEMENT_ELSE;
}

ReturnStatement::ReturnStatement(File *file, Expression *expression) {
    this->file           = file;
    this->expression     = expression;
    this->statement_type = StatementType::STATEMENT_RETURN;
}

BreakStatement::BreakStatement(File *file) {
    this->file           = file;
    this->statement_type = StatementType::STATEMENT_BREAK;
}

EnumStatement::EnumStatement(File *file, Token identifier, std::vector<EnumMember> members, u8 flag_mask) {
    this->file           = file;
    this->identifier     = identifier;
    this->members        = members;
    this->flag_mask      = flag_mask;
    this->statement_type = StatementType::STATEMENT_ENUM;
}

ContinueStatement::ContinueStatement(File *file) {
    this->file           = file;
    this->statement_type = StatementType::STATEMENT_CONTINUE;
}

AliasStatement::AliasStatement(File *file, Token identifier, TypeExpression *type_expression) {
    this->file            = file;
    this->type_expression = type_expression;
    this->identifier      = identifier;
    this->statement_type  = StatementType::STATEMENT_ALIAS;
}