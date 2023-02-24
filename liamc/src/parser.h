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
struct ContinueStatement;
struct AliasStatement;
struct TestStatement;
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
    i32 current;
    std::filesystem::path path;
    File *file;

    Parser(std::filesystem::path path, std::vector<Token> &tokens);

    void parse();

    /* statements */
    Statement *eval_statement();
    LetStatement *eval_let_statement();
    ScopeStatement *eval_scope_statement();
    StructStatement *eval_struct_statement(bool is_extern = false);
    FnStatement *eval_fn_statement(bool is_extern = false);
    ReturnStatement *eval_return_statement();
    BreakStatement *eval_break_statement();
    ImportStatement *eval_import_statement();
    ForStatement *eval_for_statement();
    IfStatement *eval_if_statement();
    ElseStatement *eval_else_statement();
    ExpressionStatement *eval_expression_statement();
    Statement *eval_extern_statement();
    EnumStatement *eval_enum_statement();
    ContinueStatement *eval_continue_statement();
    AliasStatement *eval_alias_statement();
    TestStatement *eval_test_statement();
    Statement *eval_line_starting_expression();

    /* normal expressions */
    Expression *eval_expression();
    Expression *eval_is();
    Expression *eval_propagation();
    Expression *eval_or();
    Expression *eval_and();
    Expression *eval_equality();
    Expression *eval_relational();
    Expression *eval_term();
    Expression *eval_factor();
    Expression *eval_unary();
    Expression *eval_subscript();
    Expression *eval_call();
    Expression *eval_primary();
    Expression *eval_fn();
    Expression *eval_slice();
    Expression *eval_instantiate_expression();
    Expression *eval_group_expression();

    /* return_type expressions */
    TypeExpression *eval_type_expression();
    TypeExpression *eval_type_union();
    TypeExpression *eval_type_unary();
    TypeExpression *eval_type_specified_generics();
    TypeExpression *eval_type_primary();
    TypeExpression *eval_type_fn();

    bool match(TokenType type);
    inline Token *peek(i32 offset = 0);
    Token *consume_token();
    i32 find_balance_point(TokenType push, TokenType pull, i32 from);
    Token *consume_token_of_type(TokenType type);
    std::vector<Expression *> consume_comma_seperated_arguments(TokenType closer);
    std::vector<TypeExpression *> consume_comma_seperated_types(TokenType closer);
    std::vector<Token> consume_comma_seperated_token_arguments(TokenType closer);
    std::vector<std::tuple<Token, Expression *>> consume_comma_seperated_named_arguments(TokenType closer);

    // this is what CSV is
    std::vector<std::tuple<Token, TypeExpression *>> consume_comma_seperated_params();
};