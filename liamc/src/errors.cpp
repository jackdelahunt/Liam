#include "errors.h"

#include <utility>

#include "file.h"
#include "fmt/color.h"
#include "fmt/core.h"
#include "liam.h"
#include "utils.h"

ErrorReporter *ErrorReporter::singleton = NULL;

std::string build_highlighter(u64 start, u64 length) {

    ASSERT_MSG(start > 0, "Character location should always start at 1");

    auto source = std::string();
    for (int i = 0; i < start - 1; i++)
    { source.append(" "); }

    for (int i = 0; i < length; i++)
    { source.append("^"); }

    return source;
}

void ParserError::print_error_message() {

    auto file_data = FileManager::load(&file);

    std::string top    = "";
    std::string middle = file_data->line(this->span.line);
    std::string bottom = build_highlighter(this->span.start, this->span.end - this->span.start);

    if (this->span.line > 1)
    {
        top = file_data->line(this->span.line - 1);
        rtrim(top);
    }

    rtrim(middle);

    fmt::print(stderr, fmt::emphasis::bold | fg(fmt::color::red), "ERROR :: {}\n", error);
    fmt::print(
        stderr,
        "--> {}:{}:{}\n"
        "   |   {}\n"
        "   |   {}\n"
        "   |   {}\n",
        file, this->span.line, this->span.start, top, middle, bottom
    );
}

void TypeCheckerError::print_error_message() {
    fmt::print(stderr, fmt::emphasis::bold | fg(fmt::color::red), "Type checking ERROR :: {}\n", error);

    if (this->expr_1)
    { print_error_at_span(&file, expr_1->span); }

    if (this->expr_2)
    { print_error_at_span(&file, expr_2->span); }

    if (this->type_expr_1)
    { print_error_at_span(&file, type_expr_1->span); }

    if (this->type_expr_2)
    { print_error_at_span(&file, type_expr_2->span); }
}

ErrorReporter::ErrorReporter() {
    parse_errors      = std::vector<ParserError>();
    type_check_errors = std::vector<TypeCheckerError>();
}

void ErrorReporter::report_parser_error(std::string file, Span span, std::string message) {
    if (ErrorReporter::singleton == nullptr)
    { ErrorReporter::singleton = new ErrorReporter(); }

    ErrorReporter::singleton->parse_errors.push_back(ParserError{std::move(file), span, std::move(message)});
    ErrorReporter::singleton->error_reported_since_last_check = true;
}

void ErrorReporter::report_type_checker_error(
    std::string file, Expression *expr_1, Expression *expr_2, TypeExpression *type_expr_1, TypeExpression *type_expr_2,
    std::string message
) {
    if (ErrorReporter::singleton == nullptr)
    { ErrorReporter::singleton = new ErrorReporter(); }

    ErrorReporter::singleton->type_check_errors.push_back(TypeCheckerError{
        std::move(file), expr_1, expr_2, type_expr_1, type_expr_2, std::move(message)});

    ErrorReporter::singleton->error_reported_since_last_check = true;
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

bool ErrorReporter::has_error_since_last_check() {
    if (ErrorReporter::singleton == nullptr)
        return false;

    if (ErrorReporter::singleton->error_reported_since_last_check)
    {
        ErrorReporter::singleton->error_reported_since_last_check = false;
        return true;
    }

    return false;
}

u64 ErrorReporter::error_count() {
    if (ErrorReporter::singleton == nullptr)
        return 0;

    return ErrorReporter::singleton->parse_errors.size() + ErrorReporter::singleton->type_check_errors.size();
}

void print_error_at_span(std::string *file, Span span) {
    auto file_data = FileManager::load(file);

    std::string top    = "";
    std::string middle = file_data->line(span.line);
    std::string bottom = build_highlighter(span.start, span.end - span.start);

    if (span.line > 1)
    {
        top = file_data->line(span.line - 1);
        trim(top);
    }

    rtrim(middle);

    fmt::print(
        stderr,
        "--> {}:{}:{}\n"
        "   |   {}\n"
        "   |   {}\n"
        "   |   {}\n",
        *file, span.line, span.start, top, middle, bottom
    );
}
