#pragma once

#include <string>
#include <filesystem>
#include <unordered_map>

#include "liam.h"
#include "parser.h"
#include "type_info.h"

struct Module {
    std::string name;
    std::filesystem::path path;
    std::vector<File *> files;

    std::unordered_map<std::string, TypeInfo *> builtin_type_table; // u64 string...
    std::unordered_map<std::string, TypeInfo *> top_level_type_table;         // structs
    std::unordered_map<std::string, TypeInfo *> top_level_identifier_table;   // variables or funcs

    Module(std::string name, std::filesystem::path path, std::vector<File *> files);
};