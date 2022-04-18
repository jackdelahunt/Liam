#pragma once
#include <vector>
#include <iostream>
#include <sstream>
#include "lexer.h"
#include "statement.h"
#include "expression.h"
#include "liam.h"

struct File {
    std::vector<Statement*> statements;

    File();
};

struct Parser {
    std::vector<Token> tokens;
    int current;
    File root;

    Parser(std::vector<Token>& tokens);

    void parse();
    Statement* eval_statement();
    LetStatement* eval_let_statement();
    ScopeStatement* eval_scope_statement();
    FnStatement* eval_fn_statement();
    LoopStatement* eval_loop_statement();
    int find_balance_point(TokenType push, TokenType pull, int from);
    InsertStatement* eval_insert_statement();
    ReturnStatement* eval_return_statement();
    ExpressionStatement* eval_expression_statement();
    AssigmentStatement* eval_assigment_statement();
    Expression* eval_expression();
    Expression* eval_term();
    Expression* eval_factor();
    Expression* eval_unary();
    Expression* eval_call();
    Expression* eval_primary();
    bool match(TokenType type);
    Token* peek(int offset = 0);
    Token* consume_token();
    Token* consume_token_of_type(TokenType type);
    std::vector<Expression*> consume_arguments();
    std::vector<std::tuple<Token, Token>> consume_params();

};

void unexpected_token(Token* got, TokenType expected); 
void unexpected_eof();