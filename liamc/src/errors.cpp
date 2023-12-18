#include "errors.h"

#include <format>
#include <utility>

#include "file.h"
#include "liam.h"
#include "utils.h"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define DEFAULT "\033[0m"

ErrorReporter *ErrorReporter::singleton = NULL;

void ParserError::print_error_message() {
    std::cerr << std::format("{}Parsing error :: {}{}\n", RED, error, DEFAULT);
    write_error_annotation_at_span(&this->file, this->span);
}

TypeCheckerError TypeCheckerError::make(std::string file) {
    auto error = TypeCheckerError{
        .file = std::move(file), .expr_1 = NULL, .expr_2 = NULL, .type_expr_1 = NULL, .type_expr_2 = NULL, .error = ""};

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

TypeCheckerError &TypeCheckerError::set_message(std::string message) {
    this->error = std::move(message);
    return *this;
}

void TypeCheckerError::report() {
    ErrorReporter::report_type_checker_error(*this);
}

void TypeCheckerError::print_error_message() {
    std::cerr << std::format("{}Type checking error :: {}{}\n", RED, error, DEFAULT);

    if (this->expr_1) {
        write_error_annotation_at_span(&file, this->expr_1->span);
    }

    if (this->expr_2) {
        write_error_annotation_at_span(&file, this->expr_2->span);
    }

    if (this->type_expr_1) {
        write_error_annotation_at_span(&file, this->type_expr_1->span);
    }

    if (this->type_expr_2) {
        write_error_annotation_at_span(&file, this->type_expr_2->span);
    }
}

ErrorReporter::ErrorReporter() {
    parse_errors            = std::vector<ParserError>();
    type_check_errors       = std::vector<TypeCheckerError>();
    errors_since_last_check = 0;
}

void ErrorReporter::report_parser_error(std::string file, Span span, std::string message) {
    if (ErrorReporter::singleton == nullptr) {
        ErrorReporter::singleton = new ErrorReporter();
    }

    ErrorReporter::singleton->parse_errors.push_back(ParserError{std::move(file), span, std::move(message)});
    ErrorReporter::singleton->errors_since_last_check++;
}

void ErrorReporter::report_type_checker_error(std::string file, Expression *expr_1, Expression *expr_2,
                                              TypeExpression *type_expr_1, TypeExpression *type_expr_2,
                                              std::string message) {
    if (ErrorReporter::singleton == nullptr) {
        ErrorReporter::singleton = new ErrorReporter();
    }

    ErrorReporter::singleton->type_check_errors.push_back(TypeCheckerError{.file        = std::move(file),
                                                                           .expr_1      = expr_1,
                                                                           .expr_2      = expr_2,
                                                                           .type_expr_1 = type_expr_1,
                                                                           .type_expr_2 = type_expr_2,
                                                                           .error       = std::move(message)});

    ErrorReporter::singleton->errors_since_last_check++;
}

void ErrorReporter::report_type_checker_error(TypeCheckerError error) {
    if (ErrorReporter::singleton == nullptr) {
        ErrorReporter::singleton = new ErrorReporter();
    }

    ErrorReporter::singleton->type_check_errors.push_back(error);
    ErrorReporter::singleton->errors_since_last_check++;
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

    u64 error_count = ErrorReporter::singleton->errors_since_last_check;
    return error_count > 0;
}

void ErrorReporter::reset_errors() {
    // TODO this shouod be called to reset the error state to keep compiling
    // even after we have an erroe. Right now if there is an error it will
    // stop everything. Use this for example after each fn in type checking
    // to have multiple errors
    if (ErrorReporter::singleton == nullptr)
        return;

    ErrorReporter::singleton->errors_since_last_check = 0;
}

u64 ErrorReporter::error_count() {
    if (ErrorReporter::singleton == nullptr)
        return 0;

    return ErrorReporter::singleton->parse_errors.size() + ErrorReporter::singleton->type_check_errors.size();
}

void write_error_annotation_at_span(std::string *file, Span span) {
    // TODO add locations of errors
    // right now we are not printing the line and character location
    // of the error, this is kind of okay for type errors but
    // in the parser it is a real pain
    // to do this with spans, we can store the location each of the
    // newlines in the file in a vector to then lcation the line number
    // based of the spans location, e.g.
    //  1   2   3   4   5   6   <--- where the new line is for each line
    // [0, 15, 23, 45, 56, 58]
    // compare this to the location of the span to find which it starts and
    // ends at

    ASSERT(span.start <= span.end);
    FileData *file_data = FileManager::load_relative_from_cwd(*file).value();

    // the span can point to any part of a line, maybe even and single character
    // we move the start of the span the start of whatever line it is one
    // and the same for the end, we move that to the end of the line
    // if there is no start or end then it is left as the start of the file or the end
    u64 line_start      = span.start;
    u64 line_end        = span.end;

    while (line_start != 0 && file_data->data[line_start] != '\n') {
        line_start--;
    }

    while (line_end != file_data->data_length && file_data->data[line_end] != '\n') {
        line_end++;
    }

    // the message is split into 3 sections
    // For example with the message below where we want to
    // highlight the type in the let statement
    // let x : i32 = false;
    // ... this will break down into
    // "let x : ", "i32", " = false;"
    // with the middle section being printed in red text
    // highlighting the problem
    std::string message_left, message_error, message_right;

    message_left.assign(file_data->data + line_start, span.start - line_start);
    message_error.assign(file_data->data + span.start, (span.end - span.start) + 1);
    message_right.assign(file_data->data + span.end + 1, line_end - span.end);

    ltrim(message_left);
    trim(message_error);
    rtrim(message_right);

    std::cerr << std::format("--> {}\n"
                             "   |   {}{}{}{}{}\n",
                             *file, message_left, RED, message_error, DEFAULT, message_right);
}
