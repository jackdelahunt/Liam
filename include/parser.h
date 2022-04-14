#pragma once
#include <vector>
#include <iostream>
#include <sstream>
#include "lexer.h"

namespace liam {

    struct Expression {
        virtual std::ostream& format(std::ostream& os) const {
            os << "()";
            return os;
        }
    };
    std::ostream& operator<<(std::ostream& os, const Expression& expression)
    {
        return expression.format(os);
    }

    struct BinaryExpression : Expression {
        Expression* left;
        Token op;
        Expression* right;

        BinaryExpression(Expression* left, Token op, Expression* right) {
            this->left = left;
            this->op = op;
            this->right= right;
        }

        std::ostream& format(std::ostream& os) const {
            os << "(" << *left << " " << op.string << " " << *right << ")";
            return os;
        }
    };
    std::ostream& operator<<(std::ostream& os, const BinaryExpression& expression)
    {
        return expression.format(os);
    }

    struct IntLiteralExpression : Expression {
        Token token;

        IntLiteralExpression(const Token token) {
            this->token = token;
        }

        std::ostream& format(std::ostream& os) const {
            os << "(" << token.string << ")";
            return os;
        }
    };
    std::ostream& operator<<(std::ostream& os, const IntLiteralExpression& expression)
    {
        return expression.format(os);
    }

    struct StringLiteralExpression : Expression {
        Token token;

        StringLiteralExpression(const Token token) {
            this->token = token;
        }
            
        std::ostream& format(std::ostream& os) const {
            os << "(\"" << token.string << "\")";
            return os;
        }
    };
    std::ostream& operator<<(std::ostream& os, const StringLiteralExpression& expression)
    {
        return expression.format(os);
    }

    struct Statement {
        virtual std::ostream& format(std::ostream& os) const {
            os << "()";
            return os;
        }
    };
    std::ostream& operator<<(std::ostream& os, const Statement& statement)
    {
        return statement.format(os);
    }

    struct LetStatement : Statement {
        Token identifier;
        Expression* expression;

        LetStatement(Token identifier, Expression* expression) {
            this->identifier = identifier;
            this->expression = expression;
        }

        std::ostream& format(std::ostream& os) const {
            os << "(let " << identifier.string << " " << *expression << ")";
            return os;
        }
    };
    std::ostream& operator<<(std::ostream& os, const LetStatement& statement)
    {
        return statement.format(os);
    }

    struct FnStatement : Statement {
        Token identifier;
        std::vector<Statement*> body;

        FnStatement(Token identifier, std::vector<Statement*> body) {
            this->identifier = identifier;
            this->body = body;
        }

        std::ostream& format(std::ostream& os) const {
            os << "(fn " << identifier.string;
            for (auto s_ptr : body) {
                os << *s_ptr;
            }
            os << ")";
            return os;
        }
    };
    std::ostream& operator<<(std::ostream& os, const FnStatement& statement)
    {
        return statement.format(os);
    }

    struct InsertStatement : Statement {
        Token byte_code;

        InsertStatement(Token byte_code) {
            this->byte_code = byte_code;
        }

        std::ostream& format(std::ostream& os) const {
            os << "(insert " << byte_code << ")";
            return os;
        }
    };
    std::ostream& operator<<(std::ostream& os, const InsertStatement& statement)
    {
        return statement.format(os);
    }

    struct File {
        std::vector<Statement*> statements;

        File() {
            statements = std::vector<Statement*>();
        }
    };

    struct Parser {
        std::vector<Token> tokens;
        uint32_t current;
        File root;

        Parser(std::vector<Token>& tokens) {
            this->tokens = tokens;
            this->current = 0;
            this->root = File();
        }

        void parse() {
            while (current < tokens.size()) {
                root.statements.push_back(eval_statement());
            }
        }

        Statement* eval_statement() {
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

            if(peek()->type == TOKEN_LET)
                return eval_let_statement();
            else
                return eval_insert_statement();
        }

        Statement* eval_let_statement() {
            consume_token_of_type(TOKEN_LET);
            Token* identifier = consume_token_of_type(TOKEN_IDENTIFIER);
            consume_token_of_type(TOKEN_EQUAL);
            auto expression = eval_expression();
            consume_token_of_type(TOKEN_SEMI_COLON);

            return new LetStatement(*identifier, expression);
        }

        Statement* eval_fn_statement() {
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

        int find_balance_point(TokenType push, TokenType pull, int from) {
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

        Statement* eval_insert_statement() {
            consume_token_of_type(TOKEN_INSERT);
            auto byte_code = consume_token_of_type(TOKEN_STRING_LITERAL);
            consume_token_of_type(TOKEN_SEMI_COLON);

            return new InsertStatement(*byte_code);
        }

        Expression* eval_expression() {
            return eval_binary(); // TODO: figure out  what to do here
        }

        Expression* eval_binary() {
            return eval_term();
        }

        Expression* eval_term() {
            auto expr = eval_factor();

            while (match(TokenType::TOKEN_PLUS)) {
                Token* op = consume_token();
                auto right = eval_factor();
                expr = new BinaryExpression(expr, *op, right);
            }

            return expr;
        }

        Expression* eval_factor() {
            auto expr = eval_unary();

            while (match(TokenType::TOKEN_MULT)) {
                Token* op = consume_token();
                auto right = eval_unary();
                expr = new BinaryExpression(expr, *op, right);
            }

            return expr;
        }

        Expression* eval_unary() {
            return eval_primary();
        }

        Expression* eval_primary() {
            auto token = consume_token();

            if(token->type == TokenType::TOKEN_INT_LITERAL)
                return new IntLiteralExpression(*token);
            else if(token->type == TokenType::TOKEN_STRING_LITERAL)
                return new StringLiteralExpression(*token);
            
            throw;
        }

        bool match(TokenType type) {
            if(tokens.size() > 0)
                return peek()->type == type;
        }

        Token* peek() {
            return &tokens.at(current);
        }

        Token* consume_token() {
            if (current >= tokens.size())
                panic("No more tokens to consume");

            return &tokens.at(current++);
        }

        Token* consume_token_of_type(TokenType type) {
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
    };
}