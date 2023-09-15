#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "cpp_backend.h"
#include "errors.h"
#include "lexer.h"
#include "liam.h"
#include "parser.h"
#include "type_checker.h"

CompilationUnit *lex_parse(std::filesystem::path file_path);
void type_check(CompilationUnit *file);
std::string code_gen(CompilationUnit *file);