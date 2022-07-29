#pragma once
#include <filesystem>
#include <iostream>
#include <sstream>
#include <vector>

#include "expression.h"
#include "lexer.h"
#include "liam.h"
#include "statement.h"

struct Statement;
struct LetStatement;
struct ScopeStatement;
struct StructStatement;
struct FnStatement;
struct InsertStatement;
struct ReturnStatement;
struct BreakStatement;
struct ExpressionStatement;
struct AssigmentStatement;
struct ImportStatement;
struct ForStatement;
struct IfStatement;
struct ElseStatement;
struct EnumStatement;
struct Expression;
struct TypeExpression;
struct IdentifierTypeExpression;

struct File {
    std::vector<Statement *> statements;
    std::vector<std::string> imports;
    std::filesystem::path path;

    File(std::filesystem::path path);
};

struct Parser {
    std::vector<Token> tokens;
    s32 current;
    std::filesystem::path path;
    File *file;

    Parser(std::filesystem::path path, std::vector<Token> &tokens);

    void parse();

    /* statements */
    std::tuple<Statement *, bool> eval_statement();
    std::tuple<LetStatement *, bool> eval_let_statement();
    std::tuple<ScopeStatement *, bool> eval_scope_statement();
    std::tuple<StructStatement *, bool> eval_struct_statement(bool is_extern = false);
    std::tuple<FnStatement *, bool> eval_fn_statement(bool is_extern = false);
    std::tuple<InsertStatement *, bool> eval_insert_statement();
    std::tuple<ReturnStatement *, bool> eval_return_statement();
    std::tuple<BreakStatement *, bool> eval_break_statement();
    std::tuple<ImportStatement *, bool> eval_import_statement();
    std::tuple<ForStatement *, bool> eval_for_statement();
    std::tuple<IfStatement *, bool> eval_if_statement();
    std::tuple<ElseStatement *, bool> eval_else_statement();
    std::tuple<ExpressionStatement *, bool> eval_expression_statement();
    std::tuple<Statement *, bool> eval_extern_statement();
    std::tuple<EnumStatement *, bool> eval_enum_statement();

    std::tuple<Statement *, bool> eval_line_starting_expression();

    /* normal expressions */
    std::tuple<Expression *, bool> eval_expression();
    std::tuple<Expression *, bool> eval_is();
    std::tuple<Expression *, bool> eval_or();
    std::tuple<Expression *, bool> eval_and();
    std::tuple<Expression *, bool> eval_equality();
    std::tuple<Expression *, bool> eval_relational();
    std::tuple<Expression *, bool> eval_term();
    std::tuple<Expression *, bool> eval_factor();
    std::tuple<Expression *, bool> eval_unary();
    std::tuple<Expression *, bool> eval_call();
    std::tuple<Expression *, bool> eval_primary();
    std::tuple<Expression *, bool> eval_new_expression();
    std::tuple<Expression *, bool> eval_group_expression();

    /* return_type expressions */
    std::tuple<TypeExpression *, bool> eval_type_expression();
    std::tuple<TypeExpression *, bool> eval_type_union();
    std::tuple<TypeExpression *, bool> eval_type_unary();
    std::tuple<TypeExpression *, bool> eval_type_specified_generics();
    std::tuple<IdentifierTypeExpression *, bool> eval_type_identifier();

    bool match(TokenType type);
    inline Token *peek(s32 offset = 0);
    Token *consume_token();
    s32 find_balance_point(TokenType push, TokenType pull, s32 from);
    std::tuple<Token *, bool> consume_token_of_type(TokenType type);
    std::tuple<std::vector<Expression *>, bool> consume_comma_seperated_arguments(TokenType closer);
    std::tuple<std::vector<TypeExpression *>, bool> consume_comma_seperated_types(TokenType closer);
    std::tuple<std::vector<Token>, bool> consume_comma_seperated_token_arguments(TokenType closer);
    std::tuple<std::vector<std::tuple<Token, Expression *>>, bool> consume_comma_seperated_named_arguments(
        TokenType closer
    );

    // this is what CSV is
    std::tuple<std::vector<std::tuple<Token, TypeExpression *>>, bool> consume_comma_seperated_params();
};