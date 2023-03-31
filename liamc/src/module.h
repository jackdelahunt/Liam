#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include "liam.h"
#include "parser.h"
#include "type_info.h"

struct TopLevelDescriptor {
    std::string identifier;
    TypeInfo *type_info;
};

struct Module {
    u16 module_id;
    std::string name;
    std::filesystem::path path;
    std::vector<File *> files;

    std::unordered_map<std::string, TypeInfo *> builtin_type_table;
    std::unordered_map<std::string, u64> top_level_type_table;     // module defined types
    std::unordered_map<std::string, u64> top_level_function_table; // module defined functions

    std::vector<TopLevelDescriptor> top_level_type_descriptors;
    std::vector<TopLevelDescriptor> top_level_fn_descriptors;

    Module(u16 module_id, std::string name, std::filesystem::path path, std::vector<File *> files);
    void add_type(Token idetifier, TypeInfo *type_info);
    void add_function(Token identifier, TypeInfo *type_info);
    std::tuple<TypeInfo *, bool> get_type(Token *identifier);
    std::tuple<TypeInfo *, bool> get_type(std::string identifier);
    std::tuple<TypeInfo *, bool> get_function(Token *identifier);
    std::tuple<TypeInfo *, bool> get_function(std::string identifier);
};