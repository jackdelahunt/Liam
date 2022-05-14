#pragma once
#include <vector>
#include <iostream>
#include <sstream>
#include "lexer.h"
#include "statement.h"
#include "expression.h"
#include "liam.h"
#include <filesystem>

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

struct ErrorReport {
    int line;
    int character;
    std::string error;
};

struct Parser {
    std::vector<Token> tokens;
    std::vector<ErrorReport> errors;
    int current;
    std::filesystem::path path;

    Parser(std::filesystem::path path, std::vector<Token>& tokens);

    File parse();

    /* statements */
    std::tuple<Statement*, bool> eval_statement();
    std::tuple<LetStatement*, bool> eval_let_statement();
    std::tuple<ScopeStatement*, bool> eval_scope_statement();
    std::tuple<StructStatement*, bool> eval_struct_statement();
    std::tuple<FnStatement*, bool> eval_fn_statement();
    std::tuple<LoopStatement*, bool> eval_loop_statement();
    int find_balance_point(TokenType push, TokenType pull, int from);
    std::tuple<InsertStatement*, bool> eval_insert_statement();
    std::tuple<ReturnStatement*, bool> eval_return_statement();
    std::tuple<BreakStatement*, bool> eval_break_statement();
    std::tuple<ImportStatement*, bool> eval_import_statement();
    std::tuple<ForStatement*, bool> eval_for_statement();
    std::tuple<ExpressionStatement*, bool> eval_expression_statement();
    std::tuple<AssigmentStatement*, bool> eval_assigment_statement();

    /* normal expressions */
    std::tuple<Expression*, bool> eval_expression();
    std::tuple<Expression*, bool> eval_term();
    std::tuple<Expression*, bool> eval_factor();
    std::tuple<Expression*, bool> eval_unary();
    std::tuple<Expression*, bool> eval_postfix();
    std::tuple<Expression*, bool> eval_call();
    std::tuple<Expression*, bool> eval_array();
    std::tuple<Expression*, bool> eval_primary();
    std::tuple<Expression*, bool> eval_new_expression();

    /* type expressions */
    std::tuple<TypeExpression*, bool> eval_type_expression();
    std::tuple<IdentifierTypeExpression*, bool> eval_identifier_type_expression();
    std::tuple<PointerTypeExpression*, bool> eval_pointer_type_expression();
    std::tuple<ArrayTypeExpression*, bool> eval_array_type_expression();

    void report_error(int line, int character, std::string message);
    bool match(TokenType type);
    inline Token* peek(int offset = 0);
    Token* consume_token();
    std::tuple<Token*, bool> consume_token_of_type(TokenType type);
    std::tuple<std::vector<Expression*>, bool> consume_arguments(TokenType closer);
    std::tuple<CSV, bool> consume_comma_seperated_values();

};

void unexpected_eof();