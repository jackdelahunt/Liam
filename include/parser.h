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
            os << "(" << identifier.string << " " << *expression << ")";
            return os;
        }
    };
    std::ostream& operator<<(std::ostream& os, const LetStatement& statement)
    {
        return statement.format(os);
    }

    struct Parser {
        std::vector<Token> tokens;
        uint32_t current;
        Statement* root;

        Parser(std::vector<Token>& tokens) {
            this->tokens = tokens;
            this->current = 0;
            this->root = nullptr;
        }

        void parse() {
            eval_statement();
        }

        void eval_statement() {
            root = eval_let_statement();
        }

        Statement* eval_let_statement() {
            consume_token_of_type(TOKEN_LET);
            Token* identifier = consume_token_of_type(TOKEN_IDENTIFIER);
            consume_token_of_type(TOKEN_EQUAL);
            auto expression = eval_expression();
            consume_token_of_type(TOKEN_SEMI_COLON);

            return new LetStatement(*identifier, expression);
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
                return peek().type == type;
        }

        Token& peek() {
            return tokens.at(0);
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