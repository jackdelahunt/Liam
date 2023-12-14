#pragma once
#include <vector>

#include "ast.h"
#include "compilation_unit.h"

struct Statement;
struct LetStatement;
struct ScopeStatement;
struct StructStatement;
struct FnStatement;
struct ReturnStatement;
struct BreakStatement;
struct ExpressionStatement;
struct AssigmentStatement;
struct ForStatement;
struct IfStatement;
struct ElseStatement;
struct ContinueStatement;
struct ImportStatement;
struct Expression;
struct TypeExpression;
struct IdentifierTypeExpression;
struct TypeInfo;

struct Parser {
    u64 current;
    CompilationUnit *compilation_unit;

    Parser(CompilationUnit *compilation_unit);

    void parse();

    // statements
    Statement *eval_statement();
    Statement *eval_top_level_statement();
    LetStatement *eval_let_statement();
    ScopeStatement *eval_scope_statement();
    StructStatement *eval_struct_statement();
    FnStatement *eval_fn_statement();
    ReturnStatement *eval_return_statement();
    BreakStatement *eval_break_statement();
    ForStatement *eval_for_statement();
    IfStatement *eval_if_statement();
    ElseStatement *eval_else_statement();
    ExpressionStatement *eval_expression_statement();
    ContinueStatement *eval_continue_statement();
    ImportStatement *eval_import_statement();
    Statement *eval_line_starting_expression();

    // presendence order of how we parse expressions
    Expression *eval_expression();
    Expression *eval_or();
    Expression *eval_and();
    Expression *eval_equality();
    Expression *eval_relational();
    Expression *eval_term();
    Expression *eval_factor();
    Expression *eval_unary();
    Expression *eval_postfix();
    Expression *eval_primary();

    // specific expression parsing when needed
    Expression *eval_number_literal();
    Expression *eval_string_literal();
    Expression *eval_instantiate_expression();
    Expression *eval_struct_instance_expression();
    Expression *eval_group_expression();
    Expression *eval_static_array_literal();
    Expression *eval_range_expression();

    // type expressions
    TypeExpression *eval_type_expression();
    TypeExpression *eval_type_unary();
    TypeExpression *eval_type_postfix();
    TypeExpression *eval_type_primary();
    TypeExpression *eval_type_fn();
    TypeExpression *eval_type_staic_or_slice();

    bool match(TokenType type);
    Token *peek(i32 offset = 0);
    TokenIndex consume_token_with_index();
    Option<u64> find_balance_point(TokenType push, TokenType pull, u64 from);
    TokenIndex consume_token_of_type_with_index(TokenType type);
    std::vector<Expression *> consume_comma_seperated_expressions(TokenType closer);
    std::vector<TypeExpression *> consume_comma_seperated_types(TokenType closer);
    std::vector<TokenIndex> consume_comma_seperated_token_arguments(TokenType closer);
    std::vector<std::tuple<TokenIndex, Expression *>> consume_comma_seperated_named_arguments(TokenType closer);
    u8 consume_tags();

    // this is what CSV is
    std::vector<std::tuple<TokenIndex, TypeExpression *>> consume_comma_seperated_params();
};
