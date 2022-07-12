#include "errors.h"

#include <utility>

#include "liam.h"
#include "fmt/core.h"

ErrorReporter *ErrorReporter::singleton = nullptr;

std::string ErrorReport::build_error_message() {
    std::string message = fmt::format(
        "error {}\n"
        "--> {}:{}:{}\n"
    , error, file, line, character);

    return message;
}

ErrorReporter::ErrorReporter() {
    reports = std::vector<ErrorReport>();
}

void ErrorReporter::report_error(std::string file, s32 line, s32 character, std::string message) {
    if (ErrorReporter::singleton == nullptr)
    { ErrorReporter::singleton = new ErrorReporter(); }

    ErrorReporter::singleton->reports.push_back(ErrorReport{std::move(file), line, character, std::move(message)});
}

std::vector<ErrorReport> *ErrorReporter::all_reports() {
    return &ErrorReporter::singleton->reports;
}

bool ErrorReporter::has_errors() {
    if (ErrorReporter::singleton == nullptr)
        return false;

    return !ErrorReporter::singleton->reports.empty();
}

u64 ErrorReporter::error_count() {
    return ErrorReporter::singleton->reports.size();
}
