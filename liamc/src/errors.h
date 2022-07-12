#pragma once
#include <string>
#include <utility>
#include <vector>

#include "liam.h"

struct ErrorReport {
    std::string file;
    s32 line;
    s32 character;
    std::string error;

    std::string build_error_message();
};

struct ErrorReporter {
    static ErrorReporter *singleton;
    std::vector<ErrorReport> reports;

    ErrorReporter();

    static void report_error(std::string file, s32 line, s32 character, std::string message);
    static std::vector<ErrorReport> *all_reports();
    static bool has_errors();
    static u64 error_count();
};
