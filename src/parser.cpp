#pragma once
#include "parser.h"

File::File() {
    statements = std::vector<Statement*>();
}

Parser::Parser(std::vector<Token>& tokens) {
    this->tokens = tokens;
    this->current = 0;
    this->root = File();
}

void Parser::parse() {
    while (current < tokens.size()) {
        root.statements.push_back(eval_statement());
    }
}

Statement* Parser::eval_statement() {
    switch (peek()->type)
    {
    case TOKEN_LET:
        return eval_let_statement();
        break;
    case TOKEN_FN:
        return eval_fn_statement();
        break;
    case TOKEN_INSERT:
        return eval_insert_statement();
        break;
    default:
        panic("Cannot parse token as the begining of a statement");
        break;
    }

    if (peek()->type == TOKEN_LET)
        return eval_let_statement();
    else
        return eval_insert_statement();
}

Statement* Parser::eval_let_statement() {
    consume_token_of_type(TOKEN_LET);
    Token* identifier = consume_token_of_type(TOKEN_IDENTIFIER);
    consume_token_of_type(TOKEN_EQUAL);
    auto expression = eval_expression();
    consume_token_of_type(TOKEN_SEMI_COLON);

    return new LetStatement(*identifier, expression);
}

Statement* Parser::eval_fn_statement() {
    consume_token_of_type(TOKEN_FN);
    Token* identifier = consume_token_of_type(TOKEN_IDENTIFIER);
    consume_token_of_type(TOKEN_PAREN_OPEN);
    consume_token_of_type(TOKEN_PAREN_CLOSE);
    consume_token_of_type(TOKEN_BRACE_OPEN);

    auto statements = std::vector<Statement*>();
    int closing_brace_index = find_balance_point(TOKEN_BRACE_OPEN, TOKEN_BRACE_CLOSE, current - 1);
    while (current < closing_brace_index) {
        statements.push_back(eval_statement());
    }
    consume_token_of_type(TOKEN_BRACE_CLOSE);

    return new FnStatement(*identifier, statements);
}

int Parser::find_balance_point(TokenType push, TokenType pull, int from) {
    int current_index = from;
    int balance = 0;

    while (current_index < tokens.size()) {
        if (tokens.at(current_index).type == push) {
            balance++;
            if (balance == 0)
                return current_index;
        }
        if (tokens.at(current_index).type == pull) {
            balance--;
            if (balance == 0)
                return current_index;
        }

        current_index++;
    }
}

Statement* Parser::eval_insert_statement() {
    consume_token_of_type(TOKEN_INSERT);
    auto byte_code = eval_expression();
    consume_token_of_type(TOKEN_SEMI_COLON);

    return new InsertStatement(byte_code);
}

Expression* Parser::eval_expression() {
    return eval_binary(); // TODO: figure out  what to do here
}

Expression* Parser::eval_binary() {
    return eval_term();
}

Expression* Parser::eval_term() {
    auto expr = eval_factor();

    while (match(TokenType::TOKEN_PLUS)) {
        Token* op = consume_token();
        auto right = eval_factor();
        expr = new BinaryExpression(expr, *op, right);
    }

    return expr;
}

Expression* Parser::eval_factor() {
    auto expr = eval_unary();

    while (match(TokenType::TOKEN_MULT)) {
        Token* op = consume_token();
        auto right = eval_unary();
        expr = new BinaryExpression(expr, *op, right);
    }

    return expr;
}

Expression* Parser::eval_unary() {
    return eval_primary();
}

Expression* Parser::eval_primary() {
    auto token = consume_token();

    if (token->type == TokenType::TOKEN_INT_LITERAL)
        return new IntLiteralExpression(*token);
    else if (token->type == TokenType::TOKEN_STRING_LITERAL)
        return new StringLiteralExpression(*token);

    throw;
}

bool Parser::match(TokenType type) {
    if (tokens.size() > 0)
        return peek()->type == type;
}

Token* Parser::peek() {
    return &tokens.at(current);
}

Token* Parser::consume_token() {
    if (current >= tokens.size())
        panic("No more tokens to consume");

    return &tokens.at(current++);
}

Token* Parser::consume_token_of_type(TokenType type) {
    if (current >= tokens.size()) {
        std::ostringstream oss;
        oss << "Expected " << type << " but there are no more tokens to consume";
        panic(oss.str());
    }

    auto t_ptr = &tokens.at(current++);
    if (t_ptr->type != type) {
        std::ostringstream oss;
        oss << "Expected " << type << " got " << t_ptr->type;
        panic(oss.str());
    }

    return t_ptr;
}