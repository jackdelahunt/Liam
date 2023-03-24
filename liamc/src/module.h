#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include "liam.h"
#include "parser.h"
#include "type_info.h"

struct Module {
    std::string name;
    std::filesystem::path path;
    std::vector<File *> files;

    std::unordered_map<std::string, TypeInfo *> builtin_type_table;
    std::unordered_map<std::string, TypeInfo *> top_level_type_table;     // module defined types
    std::unordered_map<std::string, TypeInfo *> top_level_function_table; // module defined functions

    Module(std::string name, std::filesystem::path path, std::vector<File *> files);
    void add_type(Token token, TypeInfo *type_info);
    void add_function(Token, TypeInfo *type_info);
    std::tuple<TypeInfo *, bool> get_type(Token *identifier);
    std::tuple<TypeInfo *, bool> get_type(std::string identifier);
};