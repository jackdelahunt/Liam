#include "errors.h"

#include <utility>
#include <fstream>
#include <sstream>
#include <iostream>

#include "liam.h"
#include "fmt/core.h"
#include "fmt/color.h"

ErrorReporter *ErrorReporter::singleton = nullptr;

void ErrorReport::print_error_message() {
    
    std::ifstream ifs(file);
	std::vector<std::string> lines;

    for (std::string line; std::getline(ifs, line); /**/) {
        lines.push_back(line);
    }

    std::string top = "";
    std::string middle = "";
    std::string bottom = "";

    if(line - 2 >= 0 && line - 2 < lines.size()) {
        top = lines.at(line - 2);
    }

    if(line - 1 >= 0 && line - 1 < lines.size()) {
        middle = lines.at(line - 1);
    }


    for(int i = 0; i < character - 1; i++) {
        bottom.append(" ");
    }
    bottom.append("^");

    fmt::print(fmt::emphasis::bold | fg(fmt::color::red), "error {}\n", error);
    fmt::print(
        "--> {}:{}:{}\n"
        "   |   {}\n"
        "   |   {}\n"
        "   |   {}\n"
    , file, line, character, top, middle, bottom
    );
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
