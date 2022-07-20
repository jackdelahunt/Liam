#include "errors.h"

#include <utility>

#include "file.h"
#include "fmt/color.h"
#include "fmt/core.h"
#include "liam.h"
#include "utils.h"

ErrorReporter *ErrorReporter::singleton = NULL;

std::string build_highlighter(u64 start, u64 length) {
    auto source = std::string();
    for (int i = 0; i < start - 1; i++)
    { source.append(" "); }

    for (int i = 0; i < length; i++)
    { source.append("^"); }

    return source;
}

void print_expression_type_error(std::string *file, Expression *expression) {
    auto file_data = FileManager::load(file);

    std::string top    = "";
    std::string middle = file_data->line(expression->span.line);
    std::string bottom = build_highlighter(expression->span.start, expression->span.end - expression->span.start);

    if (expression->span.line > 1)
    {
        top = file_data->line(expression->span.line - 1);
        trim(top);
    }

    rtrim(middle);

    fmt::print(
        "--> {}:{}:{}\n"
        "   |   {}\n"
        "   |   {}\n"
        "   |   {}\n",
        *file, expression->span.line, expression->span.start, top, middle, bottom
    );
}

void print_type_expression_type_error(std::string *file, TypeExpression *type_expression) {
    auto file_data = FileManager::load(file);

    std::string top    = "";
    std::string middle = file_data->line(type_expression->span.line);
    std::string bottom =
        build_highlighter(type_expression->span.start, type_expression->span.end - type_expression->span.start);

    if (type_expression->span.line > 1)
    {
        top = file_data->line(type_expression->span.line - 1);
        trim(top);
    }

    rtrim(middle);

    fmt::print(
        "--> {}:{}:{}\n"
        "   |   {}\n"
        "   |   {}\n"
        "   |   {}\n",
        *file, type_expression->span.line, type_expression->span.start, top, middle, bottom
    );
}

void ParserError::print_error_message() {

    auto file_data = FileManager::load(&file);

    std::string top    = "";
    std::string middle = file_data->line(line);
    std::string bottom = build_highlighter(character, 1);

    if (line > 0)
    {
        top = file_data->line(line - 1);
        rtrim(top);
    }

    rtrim(middle);

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
    fmt::print(fmt::emphasis::bold | fg(fmt::color::red), "error {}\n", error);

    if (this->expr_1)
    { print_expression_type_error(&file, expr_1); }

    if (this->expr_2)
    { print_expression_type_error(&file, expr_2); }

    if (this->type_expr_1)
    { print_type_expression_type_error(&file, type_expr_1); }

    if (this->type_expr_2)
    { print_type_expression_type_error(&file, type_expr_2); }
}

ErrorReporter::ErrorReporter() {
    parse_errors      = std::vector<ParserError>();
    type_check_errors = std::vector<TypeCheckerError>();
}

void ErrorReporter::report_parser_error(std::string file, u32 line, u32 character, std::string message) {
    if (ErrorReporter::singleton == nullptr)
    { ErrorReporter::singleton = new ErrorReporter(); }

    ErrorReporter::singleton->parse_errors.push_back(ParserError{std::move(file), line, character, std::move(message)});
}

void ErrorReporter::report_type_checker_error(
    std::string file, Expression *expr_1, Expression *expr_2, TypeExpression *type_expr_1, TypeExpression *type_expr_2,
    std::string message
) {
    if (ErrorReporter::singleton == nullptr)
    { ErrorReporter::singleton = new ErrorReporter(); }

    ErrorReporter::singleton->type_check_errors.push_back(TypeCheckerError{
        std::move(file), expr_1, expr_2, type_expr_1, type_expr_2, std::move(message)});
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
