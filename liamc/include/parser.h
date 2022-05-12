#pragma once
#include <vector>
#include <iostream>
#include <sstream>
#include "lexer.h"
#include "statement.h"
#include "expression.h"
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

#define TRY(type, value, func) \
    type value = nullptr; \
     { \
     auto [ptr, error] = func; \
        if(error) { \
            return {nullptr, true}; \
        }                \
        value = ptr; \
    }

#define NAMED_TOKEN(value, type) \
    Token* value = nullptr;                           \
    {                          \
        auto tuple = consume_token_of_type(type);                   \
        value = std::get<0>(tuple);                    \
        auto try_error = std::get<1>(tuple);                    \
        if(try_error) { \
            return {nullptr, true}; \
        }                          \
    }

#define TRY_TOKEN(type) \
    {                          \
        auto [_, _try_error_] = consume_token_of_type(type);                   \
        if(_try_error_) { \
            return {nullptr, true}; \
        }                          \
    }

#define WIN(value) \
    {value, false}

#define FAIL(line, character, message) \
    report_error(line, character, message); \
    return {nullptr, true};


struct File {
    std::vector<Statement*> statements;
    File();
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
    File root;
    std::string path;

    Parser(std::string path, std::vector<Token>& tokens);

    void parse();

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
    Token* peek(int offset = 0);
    Token* consume_token();
    std::tuple<Token*, bool> consume_token_of_type(TokenType type);
    void unexpected_token(Token* got, TokenType expected);
    std::tuple<std::vector<Expression*>, bool> consume_arguments(TokenType closer);
    std::tuple<CSV, bool> consume_comma_seperated_values();

};

void unexpected_eof();