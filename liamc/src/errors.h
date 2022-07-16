#pragma once
#include <string>
#include <utility>
#include <vector>

#include "expression.h"
#include "liam.h"

struct ParserError {
    std::string file;
    s32 line;
    s32 character;
    std::string error;

    void print_error_message();
};

struct TypeCheckerError {
    std::string file;
    Expression *expr_1;
    Expression *expr_2;
    TypeExpression *type_expr_1;
    TypeExpression *type_expr_2;
    std::string error;

    void print_error_message();
};

struct ErrorReporter {
    static ErrorReporter *singleton;
    std::vector<ParserError> parse_errors;
    std::vector<TypeCheckerError> type_check_errors;

    ErrorReporter();

    static void report_parser_error(std::string file, s32 line, s32 character, std::string message);
    static void report_type_checker_error(
        std::string file, Expression *expr_1, Expression *expr_2, TypeExpression *type_expr_1,
        TypeExpression *type_expr_2, std::string message
    );
    static bool has_parse_errors();
    static bool has_type_check_errors();
    static u64 error_count();
};
