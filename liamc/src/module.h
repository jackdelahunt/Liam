#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include "liam.h"
#include "parser.h"
#include "type_info.h"

struct Module {
    u16 module_id;
    std::string name;
    std::filesystem::path path;
    std::vector<File *> files;

    std::unordered_map<std::string, TypeInfo *> builtin_type_table;
    std::unordered_map<std::string, TypeInfo *> top_level_type_table;     // module defined types
    std::unordered_map<std::string, TypeInfo *> top_level_function_table; // module defined functions

    Module(u16 module_id, std::string name, std::filesystem::path path, std::vector<File *> files);
    TypeInfo *get_type_if_exists(Token identifier);
    void add_type(Token idetifier, TypeInfo *type_info);
    void add_function(Token identifier, TypeInfo *type_info);
    std::tuple<TypeInfo *, bool> get_type(Token *identifier);
    std::tuple<TypeInfo *, bool> get_type(std::string identifier);
};