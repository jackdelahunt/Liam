#pragma once
#include <vector>
#include <iostream>
#include "lexer.h"

namespace liam {

    enum ExpressionType {EXPRESSION_INT_LITERAL, EXPRESSION_BINARY};

    struct Expression {

        ExpressionType type;

        struct BinaryExpression {
            Expression* left;
            Token op;
            Expression* right;
        };

        union {
            Token literal;
            BinaryExpression binaryExpression;
        };

        Expression(ExpressionType type, Token token) {
            this->type = type;
            literal = token;
        }

        Expression(ExpressionType type, Expression* left, Token op, Expression* right) {
            this->type = type;
            auto be = BinaryExpression{left, op, right};
            this->binaryExpression = be;
        }

        Expression() {}
        ~Expression() {}
    };

    std::ostream& operator<<(std::ostream& os, const Expression& expression)
    {
        switch (expression.type)
        {
        case ExpressionType::EXPRESSION_INT_LITERAL:
            os << expression.literal;
            break;
        case ExpressionType::EXPRESSION_BINARY:
            os  << *expression.binaryExpression.left << " " 
                << expression.binaryExpression.op << " " 
                << *expression.binaryExpression.right;
            break;
        }
        return os;
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
            auto left = eval_primary();
            auto op = consume_token();
            auto right = eval_primary();

            auto e = new Expression(ExpressionType::EXPRESSION_BINARY, left, op, right);
            return e;
        }

        Expression* eval_primary() {
            return new Expression(ExpressionType::EXPRESSION_INT_LITERAL, consume_token());
        }

        bool match(TokenType type) {
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