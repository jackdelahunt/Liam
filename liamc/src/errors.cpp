#include "errors.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>

#include "fmt/color.h"
#include "fmt/core.h"
#include "liam.h"

ErrorReporter *ErrorReporter::singleton = nullptr;

void ParserError::print_error_message() {

    std::ifstream ifs(file);
    std::vector<std::string> lines;

    for (std::string line; std::getline(ifs, line); /**/)
    { lines.push_back(line); }

    std::string top    = "";
    std::string middle = "";
    std::string bottom = "";

    if (line - 2 >= 0 && line - 2 < lines.size())
    { top = lines.at(line - 2); }

    if (line - 1 >= 0 && line - 1 < lines.size())
    { middle = lines.at(line - 1); }

    for (int i = 0; i < character - 1; i++)
    { bottom.append(" "); }
    bottom.append("^");

    fmt::print(fmt::emphasis::bold | fg(fmt::color::red), "error {}\n", error);
    fmt::print(
        "--> {}:{}:{}\n"
        "   |   {}\n"
        "   |   {}\n"
        "   |   {}\n",
        file, line, character, top, middle, bottom
    );
}

void TypeCheckerError::print_error_message() {
    fmt::print("{}\n", error);
}

ErrorReporter::ErrorReporter() {
    parse_errors      = std::vector<ParserError>();
    type_check_errors = std::vector<TypeCheckerError>();
}

void ErrorReporter::report_parser_error(std::string file, s32 line, s32 character, std::string message) {
    if (ErrorReporter::singleton == nullptr)
    { ErrorReporter::singleton = new ErrorReporter(); }

    ErrorReporter::singleton->parse_errors.push_back(ParserError{std::move(file), line, character, std::move(message)});
}

void ErrorReporter::report_type_checker_error(
    std::string file, Expression *expr_1, Expression *expr_2, std::string message
) {
    if (ErrorReporter::singleton == nullptr)
    { ErrorReporter::singleton = new ErrorReporter(); }

    ErrorReporter::singleton->type_check_errors.push_back(TypeCheckerError{
        std::move(file), expr_1, expr_2, NULL, std::move(message)});
}

void ErrorReporter::report_type_checker_error(
    std::string file, TypeExpression *type_expr_1, Expression *expr_1, std::string message
) {
    if (ErrorReporter::singleton == nullptr)
    { ErrorReporter::singleton = new ErrorReporter(); }

    ErrorReporter::singleton->type_check_errors.push_back(TypeCheckerError{
        std::move(file), expr_1, NULL, type_expr_1, std::move(message)});
}

bool ErrorReporter::has_parse_errors() {
    if (ErrorReporter::singleton == nullptr)
        return false;

    return !(ErrorReporter::singleton->parse_errors.empty());
}

bool ErrorReporter::has_type_check_errors() {
    if (ErrorReporter::singleton == nullptr)
        return false;

    return !(ErrorReporter::singleton->type_check_errors.empty());
}

u64 ErrorReporter::error_count() {
    if (ErrorReporter::singleton == nullptr)
        return 0;

    return ErrorReporter::singleton->parse_errors.size();
}
