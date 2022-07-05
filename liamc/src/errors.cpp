#include "errors.h"

#include <utility>

#include "liam.h"

ErrorReporter *ErrorReporter::singleton = nullptr;

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
