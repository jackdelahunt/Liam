#pragma once

#include <string>

extern const char *ws;

void rtrim(std::string &s, const char *t = ws);
void ltrim(std::string &s, const char *t = ws);
void trim(std::string &s, const char *t = ws);
std::string copy_rtrim(std::string s, const char *t = ws);
std::string copy_ltrim(std::string s, const char *t = ws);
std::string copy_trim(std::string s, const char *t = ws);