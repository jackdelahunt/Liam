#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include "liam.h"
#include "parser.h"
#include "type_info.h"

struct Module;
struct File;

struct TopLevelDescriptor {
    std::string identifier;
    Module *module;
    File *file;
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
    void add_type(Module *module,
    File *file, Token idetifier, TypeInfo *type_info);
    void add_function(Module *module,
    File *file, Token identifier, TypeInfo *type_info);
    std::tuple<TypeInfo *, bool> get_type(Token *identifier);
    std::tuple<TypeInfo *, bool> get_type(std::string identifier);
    std::tuple<TypeInfo *, bool> get_function(Token *identifier);
    std::tuple<TypeInfo *, bool> get_function(std::string identifier);
};