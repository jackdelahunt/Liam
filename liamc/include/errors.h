#pragma once
#include <string>
#include <utility>
#include <vector>

struct ErrorReport {
    std::string file;
    int line;
    int character;
    std::string error;
};

struct ErrorReporter {
    static ErrorReporter* singleton;
    std::vector<ErrorReport> reports;

    ErrorReporter();

    static void report_error(std::string file, int line, int character, std::string message);
    static std::vector<ErrorReport>* all_reports();
    static bool has_errors();
};
