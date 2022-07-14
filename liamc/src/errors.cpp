#include "errors.h"

#include <utility>

#include "file.h"
#include "fmt/color.h"
#include "fmt/core.h"
#include "liam.h"
#include "strings.h"

ErrorReporter *ErrorReporter::singleton = NULL;

void ParserError::print_error_message() {

    auto file_data = FileManager::load(&file);

    std::string top    = "";
    std::string middle = file_data->line(line);
    std::string bottom = "";

    if(line > 0) {
        top = file_data->line(line - 1);
        rtrim(top);
    }

    rtrim(middle);

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
    fmt::print(fmt::emphasis::bold | fg(fmt::color::red), "error {}\n", error);

    if(this->expr_1) {
        fmt::print(
            "--> {}:{}:{}\n"
            "   |   {}\n"
            "   |   {}\n"
            "   |   {}\n",
            file, expr_1->span.line, expr_1->span.start, "", "", ""
        );
    }

    if(this->expr_2) {
        fmt::print(
            "--> {}:{}:{}\n"
            "   |   {}\n"
            "   |   {}\n"
            "   |   {}\n",
            file, expr_2->span.line, expr_2->span.start, "", "", ""
        );
    }

    if(this->type_expr_1) {
        fmt::print(
            "--> {}:{}:{}\n"
            "   |   {}\n"
            "   |   {}\n"
            "   |   {}\n",
            file, type_expr_1->span.line, type_expr_1->span.start, "", "", ""
        );
    }
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
