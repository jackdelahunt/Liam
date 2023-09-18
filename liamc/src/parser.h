#pragma once
#include <filesystem>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "ast.h"
#include "compilation_unit.h"
#include "lexer.h"

struct Statement;
struct LetStatement;
struct ScopeStatement;
struct StructStatement;
struct FnStatement;
struct ReturnStatement;
struct BreakStatement;
struct ExpressionStatement;
struct AssigmentStatement;
struct ImportStatement;
struct ForStatement;
struct IfStatement;
struct ElseStatement;
struct ContinueStatement;
struct Expression;
struct TypeExpression;
struct IdentifierTypeExpression;
struct TypeInfo;

struct Parser {
    u64 current;
    CompilationUnit *compilation_unit;

    Parser(CompilationUnit *compilation_unit);

    void parse();

    /* statements */
    Statement *eval_statement();
    Statement *eval_top_level_statement();
    LetStatement *eval_let_statement();
    ScopeStatement *eval_scope_statement();
    StructStatement *eval_struct_statement();
    FnStatement *eval_fn_statement();
    ReturnStatement *eval_return_statement();
    BreakStatement *eval_break_statement();
    ImportStatement *eval_import_statement();
    ForStatement *eval_for_statement();
    IfStatement *eval_if_statement();
    ElseStatement *eval_else_statement();
    ExpressionStatement *eval_expression_statement();
    ContinueStatement *eval_continue_statement();
    Statement *eval_line_starting_expression();

    /* normal expressions */
    Expression *eval_expression();
    Expression *eval_or();
    Expression *eval_and();
    Expression *eval_equality();
    Expression *eval_relational();
    Expression *eval_term();
    Expression *eval_factor();
    Expression *eval_unary();
    Expression *eval_subscript();
    Expression *eval_postfix();
    Expression *eval_primary();
    Expression *eval_string_literal();
    Expression *eval_fn();
    Expression *eval_slice_literal();
    Expression *eval_instantiate_expression();
    Expression *eval_struct_instance_expression();
    Expression *eval_group_expression();

    /* return_type expressions */
    TypeExpression *eval_type_expression();
    TypeExpression *eval_type_unary();
    TypeExpression *eval_type_specified_generics();
    TypeExpression *eval_type_primary();
    TypeExpression *eval_type_fn();

    bool match(TokenType type);
    inline Token *peek(i32 offset = 0);
    Token *consume_token();
    i32 find_balance_point(TokenType push, TokenType pull, i32 from);
    Token *consume_token_of_type(TokenType type);
    TokenIndex consume_token_of_type_with_index(TokenType type);
    TokenData *get_token_data(TokenIndex token_index);
    std::vector<Expression *> consume_comma_seperated_arguments(TokenType closer);
    std::vector<TypeExpression *> consume_comma_seperated_types(TokenType closer);
    std::vector<Token> consume_comma_seperated_token_arguments(TokenType closer);
    std::vector<std::tuple<Token, Expression *>> consume_comma_seperated_named_arguments(TokenType closer);
    u8 consume_tags();

    // this is what CSV is
    std::vector<std::tuple<Token, TypeExpression *>> consume_comma_seperated_params();
};
