#pragma once
#include <vector>
#include <iostream>
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

        IntLiteralExpression(const Token& token) {
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

        StringLiteralExpression(const Token& token) {
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

    struct Parser {
        std::vector<Token> tokens;
        Expression* root;

        Parser(std::vector<Token>& tokens) {
            this->tokens = tokens;
        }

        void parse() {
            eval_expression();
        }

        void eval_expression() {
            root = eval_binary();
        }

        Expression* eval_binary() {
            return eval_term();
        }

        Expression* eval_term() {
            auto expr = eval_factor();

            while (match(TokenType::TOKEN_PLUS)) {
                Token op = consume_token();
                auto right = eval_factor();
                expr = new BinaryExpression(expr, op, right);
            }

            return expr;
        }

        Expression* eval_factor() {
            auto expr = eval_unary();

            while (match(TokenType::TOKEN_MULT)) {
                Token op = consume_token();
                auto right = eval_unary();
                expr = new BinaryExpression(expr, op, right);
            }

            return expr;
        }

        Expression* eval_unary() {
            return eval_primary();
        }

        Expression* eval_primary() {
            auto token = consume_token();

            if(token.type == TokenType::TOKEN_INT_LITERAL)
                return new IntLiteralExpression(token);
            else if(token.type == TokenType::TOKEN_STRING_LITERAL)
                return new StringLiteralExpression(token);
            
            throw;
        }

        bool match(TokenType type) {
            if(tokens.size() > 0)
                return peek().type == type;
        }

        Token& peek() {
            return tokens.at(0);
        }

        Token consume_token() {
            auto token = tokens.front();
            tokens.erase(tokens.begin());
            return token;
        }
    };
}