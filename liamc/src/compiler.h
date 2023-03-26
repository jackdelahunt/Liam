#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "cpp_backend.h"
#include "errors.h"
#include "lexer.h"
#include "liam.h"
#include "module.h"
#include "parser.h"
#include "type_checker.h"

Module *create_lex_and_parse_module_from_path(
    u16 module_id, std::string module_name, std::filesystem::path module_path
);
std::vector<Module *> lex_parse(std::filesystem::path starting_import_path);
void type_check(std::vector<Module *> *modules);
std::string code_gen(std::vector<Module *> *modules);