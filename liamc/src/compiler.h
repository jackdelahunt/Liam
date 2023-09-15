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

File * lex_parse(std::filesystem::path file_path);
void type_check(File *file);
std::string code_gen(File *file);