#include "errors.h"

#include <utility>

#include "file.h"
#include "fmt/color.h"
#include "fmt/core.h"
#include "liam.h"
#include "utils.h"

ErrorReporter *ErrorReporter::singleton = NULL;

void ParserError::print_error_message() {
    fmt::print(stderr, fmt::emphasis::bold | fg(fmt::color::red), "Parsing ERROR :: {}\n", error);
    write_error_annotation_at_span(&this->file, this->span);
}

TypeCheckerError TypeCheckerError::make(std::string file) {
    auto error = TypeCheckerError{
        .file           = std::move(file),
        .expr_1         = NULL,
        .expr_2         = NULL,
        .type_expr_1    = NULL,
        .type_expr_2    = NULL,
        .related_tokens = std::vector<Token>(),
        .error          = ""};

    return error;
}

TypeCheckerError &TypeCheckerError::set_expr_1(Expression *expression) {
    this->expr_1 = expression;
    return *this;
}

TypeCheckerError &TypeCheckerError::set_expr_2(Expression *expression) {
    this->expr_2 = expression;
    return *this;
}

TypeCheckerError &TypeCheckerError::set_type_expr_1(TypeExpression *type_expression) {
    this->type_expr_1 = type_expression;
    return *this;
}

TypeCheckerError &TypeCheckerError::set_type_expr_2(TypeExpression *type_expression) {
    this->type_expr_2 = type_expression;
    return *this;
}

TypeCheckerError &TypeCheckerError::add_related_token(Token token) {
    this->related_tokens.push_back(token);
    return *this;
}

TypeCheckerError &TypeCheckerError::set_message(std::string message) {
    this->error = std::move(message);
    return *this;
}

void TypeCheckerError::report() {
    ErrorReporter::report_type_checker_error(*this);
}

void TypeCheckerError::print_error_message() {
    fmt::print(stderr, fmt::emphasis::bold | fg(fmt::color::red), "Type checking ERROR :: {}\n", error);

    if (this->expr_1)
    { write_error_annotation_at_span(&file, this->expr_1->span); }

    if (this->expr_2)
    { write_error_annotation_at_span(&file, this->expr_2->span); }

    if (this->type_expr_1)
    { write_error_annotation_at_span(&file, this->type_expr_1->span); }

    if (this->type_expr_2)
    { write_error_annotation_at_span(&file, this->type_expr_2->span); }

    for (auto &token : this->related_tokens)
    { write_error_annotation_at_span(&file, token.span); }
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
    std::vector<Token> related_tokens, std::string message
) {
    if (ErrorReporter::singleton == nullptr)
    { ErrorReporter::singleton = new ErrorReporter(); }

    ErrorReporter::singleton->type_check_errors.push_back(TypeCheckerError{
        .file           = std::move(file),
        .expr_1         = expr_1,
        .expr_2         = expr_2,
        .type_expr_1    = type_expr_1,
        .type_expr_2    = type_expr_2,
        .related_tokens = std::move(related_tokens),
        .error          = std::move(message)});

    ErrorReporter::singleton->error_reported_since_last_check = true;
}

void ErrorReporter::report_type_checker_error(TypeCheckerError error) {
    if (ErrorReporter::singleton == nullptr)
    { ErrorReporter::singleton = new ErrorReporter(); }

    ErrorReporter::singleton->type_check_errors.push_back(error);
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

void write_error_annotation_at_span(std::string *file, Span span) {
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

std::string build_highlighter(u64 start, u64 length) {

    ASSERT_MSG(start > 0, "Character location should always start at 1");

    auto source = std::string();
    for (int i = 0; i < start - 1; i++)
    { source.append(" "); }

    for (int i = 0; i < length; i++)
    { source.append("^"); }

    return source;
}
