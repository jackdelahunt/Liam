#pragma once
#include <vector>
#include <iostream>
#include <sstream>
#include "lexer.h"
#include "statement.h"
#include "expression.h"
#include <filesystem>
#include "liam.h"

struct Statement;
struct LetStatement;
struct ScopeStatement;
struct StructStatement;
struct FnStatement;
struct LoopStatement;
struct InsertStatement;
struct ReturnStatement;
struct BreakStatement;
struct ExpressionStatement;
struct AssigmentStatement;
struct Expression;
struct TypeExpression;
struct IdentifierTypeExpression;

struct File {
    std::vector<Statement*> statements;
    std::vector<std::string> imports;
    std::filesystem::path path;

    File(std::filesystem::path path);
};

struct Parser {
    std::vector<Token> tokens;
    s32 current;
    std::filesystem::path path;

    Parser(std::filesystem::path path, std::vector<Token>& tokens);

    File parse();

    /* statements */
    std::tuple<Statement*, bool> eval_statement();
    std::tuple<LetStatement*, bool> eval_let_statement();
    std::tuple<OverrideStatement*, bool> eval_override_statement();
    std::tuple<ScopeStatement*, bool> eval_scope_statement();
    std::tuple<StructStatement*, bool> eval_struct_statement();
    std::tuple<FnStatement*, bool> eval_fn_statement();
    std::tuple<LoopStatement*, bool> eval_loop_statement();
    s32 find_balance_point(TokenType push, TokenType pull, s32 from);
    std::tuple<InsertStatement*, bool> eval_insert_statement();
    std::tuple<ReturnStatement*, bool> eval_return_statement();
    std::tuple<BreakStatement*, bool> eval_break_statement();
    std::tuple<ImportStatement*, bool> eval_import_statement();
    std::tuple<ForStatement*, bool> eval_for_statement();
    std::tuple<IfStatement*, bool> eval_if_statement();
    std::tuple<ExpressionStatement*, bool> eval_expression_statement();
    std::tuple<AssigmentStatement*, bool> eval_assigment_statement();

    /* normal expressions */
    std::tuple<Expression*, bool> eval_expression();
    std::tuple<Expression*, bool> eval_or();
    std::tuple<Expression*, bool> eval_and();
    std::tuple<Expression*, bool> eval_comparison();
    std::tuple<Expression*, bool> eval_term();
    std::tuple<Expression*, bool> eval_factor();
    std::tuple<Expression*, bool> eval_unary();
    std::tuple<Expression*, bool> eval_call();
    std::tuple<Expression*, bool> eval_primary();
    std::tuple<Expression*, bool> eval_new_expression();
    std::tuple<Expression*, bool> eval_group_expression();

    /* return_type expressions */
    std::tuple<TypeExpression*, bool> eval_type_expression();
    std::tuple<IdentifierTypeExpression*, bool> eval_identifier_type_expression();
    std::tuple<PointerTypeExpression*, bool> eval_pointer_type_expression();

    bool match(TokenType type);
    inline Token* peek(s32 offset = 0);
    Token* consume_token();
    std::tuple<Token*, bool> consume_token_of_type(TokenType type);
    std::tuple<std::vector<Expression*>, bool> consume_comma_seperated_arguments(TokenType closer);
    std::tuple<std::vector<TypeExpression*>, bool> consume_comma_seperated_types(TokenType closer);
    std::tuple<std::vector<Token>, bool> consume_comma_seperated_token_arguments(TokenType closer);
    std::tuple<CSV, bool> consume_comma_seperated_params();
};