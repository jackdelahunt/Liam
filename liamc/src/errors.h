#pragma once
#include <string>
#include <utility>
#include <vector>

#include "expression.h"
#include "liam.h"

struct ParserError {
    std::string file;
    Span span;
    std::string error;

    void print_error_message();
};

struct TypeCheckerError {
    std::string file;
    Expression *expr_1;
    Expression *expr_2;
    TypeExpression *type_expr_1;
    TypeExpression *type_expr_2;
    std::vector<Token> related_tokens;
    std::string error;

    static TypeCheckerError make(std::string file);
    TypeCheckerError &set_expr_1(Expression *expression);
    TypeCheckerError &set_expr_2(Expression *expression);
    TypeCheckerError &set_type_expr_1(TypeExpression *type_expression);
    TypeCheckerError &set_type_expr_2(TypeExpression *type_expression);
    TypeCheckerError &add_related_token(Token token);
    TypeCheckerError &set_message(std::string message);

    void print_error_message();
};

struct ErrorReporter {
    static ErrorReporter *singleton;
    std::vector<ParserError> parse_errors;
    std::vector<TypeCheckerError> type_check_errors;
    bool error_reported_since_last_check = false;

    ErrorReporter();

    static void report_parser_error(std::string file, Span span, std::string message);
    static void report_type_checker_error(
        std::string file, Expression *expr_1, Expression *expr_2, TypeExpression *type_expr_1,
        TypeExpression *type_expr_2, std::vector<Token> related_tokens, std::string message
    );
    static void report_type_checker_error(TypeCheckerError error);
    static bool has_parse_errors();
    static bool has_type_check_errors();
    static bool has_error_since_last_check();
    static u64 error_count();
};

void write_error_annotation_at_span(std::string *file, Span span);
std::string build_highlighter(u64 start, u64 length);