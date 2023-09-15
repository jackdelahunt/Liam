#include "ast.h"

#include <tuple>

VoidTypeInfo::VoidTypeInfo() {
    this->type = TypeInfoType::VOID;
}

NumberTypeInfo::NumberTypeInfo(size_t size, NumberType number_type) {
    this->size        = size;
    this->number_type = number_type;
    this->type        = TypeInfoType::NUMBER;
}

BoolTypeInfo::BoolTypeInfo() {
    this->type = TypeInfoType::BOOLEAN;
}

PointerTypeInfo::PointerTypeInfo(TypeInfo *to) {
    this->to   = to;
    this->type = TypeInfoType::POINTER;
}

PointerSliceTypeInfo::PointerSliceTypeInfo(TypeInfo *to) {
    this->to   = to;
    this->type = TypeInfoType::POINTER_SLICE;
}

StrTypeInfo::StrTypeInfo() {
    this->type = TypeInfoType::STRING;
}

TypeTypeInfo::TypeTypeInfo() {
}

StructTypeInfo::StructTypeInfo(
    u8 flag_mask, std::vector<std::tuple<std::string, FnTypeInfo *>> memberFunctions,
    std::vector<std::tuple<std::string, TypeInfo *>> members, u64 genericCount
) {
    this->flag_mask        = flag_mask;
    this->member_functions = memberFunctions;
    this->members          = members;
    this->generic_count    = genericCount;
    this->type             = TypeInfoType::STRUCT;
}

StructInstanceTypeInfo::StructInstanceTypeInfo(StructTypeInfo *structType, std::vector<TypeInfo *> genericTypes) {
    this->struct_type   = structType;
    this->generic_types = genericTypes;
    this->type          = TypeInfoType::STRUCT_INSTANCE;
}

FnTypeInfo::FnTypeInfo(
    u8 flag_mask, StructTypeInfo *parentType, TypeInfo *returnType, std::vector<TypeInfo *> genericTypeInfos,
    std::vector<TypeInfo *> args
) {
    this->flag_mask          = flag_mask;
    this->parent_type        = parentType;
    this->return_type        = returnType;
    this->generic_type_infos = genericTypeInfos;
    this->args               = args;
    this->type               = TypeInfoType::FN;
}

FnExpressionTypeInfo::FnExpressionTypeInfo(TypeInfo *returnType, std::vector<TypeInfo *> args) {
    this->return_type = returnType;
    this->args        = args;
    this->type        = TypeInfoType::FN_EXPRESSION;
}

GenericTypeInfo::GenericTypeInfo(u64 id) {
    this->id   = id;
    this->type = TypeInfoType::GENERIC;
}

EnumTypeInfo::EnumTypeInfo(std::vector<EnumMember> members, u8 flag_mask) {
    this->members   = members;
    this->flag_mask = flag_mask;
    this->type      = TypeInfoType::ENUM;
}

EnumMemberPatternMatch::EnumMemberPatternMatch(Token identifier, std::vector<Token> matched_members) {
    this->identifier        = identifier;
    this->matched_members   = matched_members;
    this->enum_member_index = -1;
}

std::ostream &Statement::format(std::ostream &os) const {
    os << "()";
    return os;
}

ExpressionStatement::ExpressionStatement(CompilationUnit *file, Expression *expression) {
    this->file           = file;
    this->expression     = expression;
    this->statement_type = StatementType::STATEMENT_EXPRESSION;
}

LetStatement::LetStatement(CompilationUnit *file, Token identifier, Expression *expression, TypeExpression *type) {
    this->identifier     = identifier;
    this->rhs            = expression;
    this->type           = type;
    this->statement_type = StatementType::STATEMENT_LET;
}

FnStatement::FnStatement(
    CompilationUnit *file, TypeExpression *parent_type, Token identifier, std::vector<Token> generics, CSV params,
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

StructStatement::StructStatement(
    CompilationUnit *file, Token identifier, std::vector<Token> generics, CSV members, u8 flag_mask
) {
    this->file           = file;
    this->identifier     = identifier;
    this->generics       = generics;
    this->members        = members;
    this->flag_mask      = flag_mask;
    this->statement_type = StatementType::STATEMENT_STRUCT;
}

AssigmentStatement::AssigmentStatement(CompilationUnit *file, Expression *lhs, ExpressionStatement *assigned_to) {
    this->file           = file;
    this->lhs            = lhs;
    this->assigned_to    = assigned_to;
    this->statement_type = StatementType::STATEMENT_ASSIGNMENT;
}

ScopeStatement::ScopeStatement(CompilationUnit *file, std::vector<Statement *> statements) {
    this->file           = file;
    this->statements     = statements;
    this->statement_type = StatementType::STATEMENT_SCOPE;
}

ImportStatement::ImportStatement(CompilationUnit *file, StringLiteralExpression *path) {
    this->file           = file;
    this->path           = path;
    this->statement_type = StatementType::STATEMENT_IMPORT;
}

ForStatement::ForStatement(
    CompilationUnit *file, Statement *assign, Expression *condition, Statement *update, ScopeStatement *body
) {
    this->file           = file;
    this->assign         = assign;
    this->condition      = condition;
    this->update         = update;
    this->body           = body;
    this->statement_type = StatementType::STATEMENT_FOR;
}

IfStatement::IfStatement(
    CompilationUnit *file, Expression *expression, ScopeStatement *body, ElseStatement *else_statement
) {
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

ReturnStatement::ReturnStatement(CompilationUnit *file, Expression *expression) {
    this->file           = file;
    this->expression     = expression;
    this->statement_type = StatementType::STATEMENT_RETURN;
}

BreakStatement::BreakStatement(CompilationUnit *file) {
    this->file           = file;
    this->statement_type = StatementType::STATEMENT_BREAK;
}

EnumMember::EnumMember(Token identifier, std::vector<TypeExpression *> members) {
    this->identifier = identifier;
    this->members    = members;
}

EnumStatement::EnumStatement(CompilationUnit *file, Token identifier, std::vector<EnumMember> members, u8 flag_mask) {
    this->file           = file;
    this->identifier     = identifier;
    this->members        = members;
    this->flag_mask      = flag_mask;
    this->statement_type = StatementType::STATEMENT_ENUM;
}

MatchStatement::MatchStatement(
    Expression *matching_expression, std::vector<EnumMemberPatternMatch> pattern_matches,
    std::vector<ScopeStatement *> pattern_match_arms
) {
    this->matching_expression = matching_expression;
    this->pattern_matches     = pattern_matches;
    this->pattern_match_arms  = pattern_match_arms;
    this->statement_type      = StatementType::STATEMENT_MATCH;
}

ContinueStatement::ContinueStatement(CompilationUnit *file) {
    this->file           = file;
    this->statement_type = StatementType::STATEMENT_CONTINUE;
}

std::ostream &Expression::format(std::ostream &os) const {
    os << "()";
    return os;
}

BinaryExpression::BinaryExpression(Expression *left, Token op, Expression *right) {
    this->left  = left;
    this->op    = op;
    this->right = right;
    this->type  = ExpressionType::EXPRESSION_BINARY;
}

UnaryExpression::UnaryExpression(Expression *expression, Token op) {
    this->expression = expression;
    this->op         = op;
    this->type       = ExpressionType::EXPRESSION_UNARY;
    this->span       = Span{.line = expression->span.line, .start = op.span.start, .end = expression->span.end};
}

SubscriptExpression::SubscriptExpression(Expression *lhs, Expression *expression) {
    this->expression = expression;
    this->lhs        = lhs;
    this->span       = lhs->span;
    this->type       = ExpressionType::EXPRESSION_SUBSCRIPT;
}

NumberLiteralExpression::NumberLiteralExpression(Token token) {
    this->token = token;
    this->type  = ExpressionType::EXPRESSION_NUMBER_LITERAL;
    this->span  = token.span;
}

StringLiteralExpression::StringLiteralExpression(Token token) {
    this->token = token;
    this->type  = ExpressionType::EXPRESSION_STRING_LITERAL;
    this->span  = Span{.line = token.span.line, .start = token.span.start, .end = token.span.end};
}

BoolLiteralExpression::BoolLiteralExpression(Token value) {
    this->value = value;
    this->type  = ExpressionType::EXPRESSION_BOOL_LITERAL;
    this->span  = Span{.line = value.span.line, .start = value.span.start, .end = value.span.end};
}

IdentifierExpression::IdentifierExpression(const Token identifier) {
    this->identifier = identifier;
    this->type       = ExpressionType::EXPRESSION_IDENTIFIER;
    this->span       = identifier.span;
}

CallExpression::CallExpression(
    Expression *identifier, std::vector<Expression *> args, std::vector<TypeExpression *> generics
) {
    this->callee   = identifier;
    this->args     = args;
    this->generics = generics;
    this->type     = ExpressionType::EXPRESSION_CALL;
    this->span     = identifier->span;
}

GetExpression::GetExpression(Expression *expression, Token member) {
    this->lhs    = expression;
    this->member = member;
    this->type   = ExpressionType::EXPRESSION_GET;
    this->span   = Span{expression->span.line, expression->span.start, member.span.end};
}

GroupExpression::GroupExpression(Expression *expression) {
    this->expression = expression;
    this->type       = ExpressionType::EXPRESSION_GROUP;
    this->span       = expression->span;
}

NullLiteralExpression::NullLiteralExpression(Token token) {
    this->type = ExpressionType::EXPRESSION_NULL_LITERAL;
    this->span = token.span;
}

ZeroLiteralExpression::ZeroLiteralExpression(Token token) {
    this->type = ExpressionType::EXPRESSION_ZERO_LITERAL;
    this->span = token.span;
}

FnExpression::FnExpression(
    std::vector<std::tuple<Token, TypeExpression *>> params, TypeExpression *return_type, ScopeStatement *body
) {
    this->params      = std::move(params);
    this->return_type = return_type;
    this->body        = body;
    this->type        = ExpressionType::EXPRESSION_FN;
    this->span        = return_type->span; // FIXME: use a better place to put the span for the fn expression
}

SliceLiteralExpression::SliceLiteralExpression(TypeExpression *type_expression, std::vector<Expression *> expressions) {
    this->type_expression = type_expression;
    this->expressions     = expressions;
    this->type            = ExpressionType::EXPRESSION_SLICE_LITERAL;
    this->span            = type_expression->span;
}

InstantiateExpression::InstantiateExpression(Expression *expression) {
    this->expression = expression;
    this->span       = expression->span;
    this->type       = ExpressionType::EXPRESSION_INSTANTIATION;
}

StructInstanceExpression::StructInstanceExpression(
    Token identifier, std::vector<TypeExpression *> generics,
    std::vector<std::tuple<Token, Expression *>> named_expressions
) {
    this->identifier        = identifier;
    this->generics          = generics;
    this->named_expressions = named_expressions;
    this->span              = identifier.span;
    this->type              = ExpressionType::EXPRESSION_STRUCT_INSTANCE;
}

EnumInstanceExpression::EnumInstanceExpression(Token lhs, Token member, std::vector<Expression *> arguments) {
    this->lhs          = lhs;
    this->member       = member;
    this->arguments    = std::move(arguments);
    this->member_index = -1; // yes I am setting a u64 to -1, lulul
    this->span         = lhs.span;
    this->type         = ExpressionType::EXPRESSION_ENUM_INSTANCE;
}

std::ostream &TypeExpression::format(std::ostream &os) const {
    os << "()";
    return os;
}

std::ostream &operator<<(std::ostream &os, const TypeExpression &expression) {
    return expression.format(os);
}

IdentifierTypeExpression::IdentifierTypeExpression(Token identifier) {
    this->identifier = identifier;
    this->type       = TypeExpressionType::TYPE_IDENTIFIER;
    this->span       = identifier.span;
}

UnaryTypeExpression::UnaryTypeExpression(UnaryType unary_type, TypeExpression *type_expression) {
    this->unary_type      = unary_type;
    this->type_expression = type_expression;
    this->type            = TypeExpressionType::TYPE_UNARY;
    this->span            = type_expression->span;
}

SpecifiedGenericsTypeExpression::SpecifiedGenericsTypeExpression(
    TypeExpression *struct_type, std::vector<TypeExpression *> generics
) {
    this->struct_type = struct_type;
    this->generics    = generics;
    this->span = Span{.line = struct_type->span.line, .start = struct_type->span.start, .end = struct_type->span.end};
    this->type = TypeExpressionType::TYPE_SPECIFIED_GENERICS;
}

FnTypeExpression::FnTypeExpression(std::vector<TypeExpression *> params, TypeExpression *return_type) {
    this->params      = std::move(params);
    this->return_type = return_type;
    this->span        = return_type->span; // FIXME: this should be a better span, return type is just used for now
    this->type        = TypeExpressionType::TYPE_FN;
}
