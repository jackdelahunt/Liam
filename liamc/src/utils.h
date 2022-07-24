#pragma once

#include "lexer.h"
#include "liam.h"
#include <string>
#include <tuple>

extern const char *ws;

bool is_digit(char c);
std::tuple<s64, NumberType, s32> extract_number_literal_size(std::string literal);
void rtrim(std::string &s, const char *t = ws);
void ltrim(std::string &s, const char *t = ws);
void trim(std::string &s, const char *t = ws);
std::string copy_rtrim(std::string s, const char *t = ws);
std::string copy_ltrim(std::string s, const char *t = ws);
std::string copy_trim(std::string s, const char *t = ws);