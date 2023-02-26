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

    fmt::print(stderr, fmt::emphasis::bold | fg(fmt::color::red), "ERROR :: {}\n", error);
    fmt::print(
        stderr,
        "--> {}:{}:{}\n"
        "   |   {}\n"
        "   |   {}\n"
        "   |   {}\n",
        file, line, character, top, middle, bottom
    );
}

void TypeCheckerError::print_error_message() {
    fmt::print(stderr, fmt::emphasis::bold | fg(fmt::color::red), "ERROR :: {}\n", error);

    if (this->expr_1)
    { print_error_at_span(&file, expr_1->span); }

    if (this->expr_2)
    { print_error_at_span(&file, expr_2->span); }

    if (this->type_expr_1)
    { print_error_at_span(&file, type_expr_1->span); }

    if (this->type_expr_2)
    { print_error_at_span(&file, type_expr_2->span); }
}

void BorrowCheckerError::print_error_message() {
    fmt::print(stderr, fmt::emphasis::bold | fg(fmt::color::red), "ERROR :: {}\n", error);

    ASSERT(this->use_after_move)
    ASSERT(this->move_of_value)

    fmt::print(stderr, fmt::emphasis::italic | fg(fmt::color::cornflower_blue), "First entered scope here\n");
    print_error_at_span(&file, this->ownership_of_value);

    fmt::print(stderr, fmt::emphasis::italic | fg(fmt::color::cornflower_blue), "Then first moved out of scope here\n");
    print_error_at_span(&file, this->move_of_value->span);

    fmt::print(
        stderr, fmt::emphasis::italic | fg(fmt::color::cornflower_blue), "Trying to use/move already moved value here\n"
    );
    print_error_at_span(&file, this->use_after_move->span);
}

ErrorReporter::ErrorReporter() {
    parse_errors      = std::vector<ParserError>();
    type_check_errors = std::vector<TypeCheckerError>();
}

void ErrorReporter::report_parser_error(std::string file, u32 line, u32 character, std::string message) {
    if (ErrorReporter::singleton == nullptr)
    { ErrorReporter::singleton = new ErrorReporter(); }

    ErrorReporter::singleton->parse_errors.push_back(ParserError{std::move(file), line, character, std::move(message)});
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

void ErrorReporter::report_borrow_checker_error(
    std::string file, Span ownership_of_value, Expression *move_of_value, Expression *use_after_move,
    std::string message
) {
    if (ErrorReporter::singleton == nullptr)
    { ErrorReporter::singleton = new ErrorReporter(); }

    ErrorReporter::singleton->borrow_check_errors.push_back(BorrowCheckerError{
        std::move(file), ownership_of_value, move_of_value, use_after_move, std::move(message)});

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

bool ErrorReporter::has_borrow_check_errors() {
    if (ErrorReporter::singleton == nullptr)
        return false;

    return !(ErrorReporter::singleton->borrow_check_errors.empty());
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

    return ErrorReporter::singleton->parse_errors.size() + ErrorReporter::singleton->type_check_errors.size() +
           ErrorReporter::singleton->borrow_check_errors.size();
}
