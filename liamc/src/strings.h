#pragma once

#include <string>

const char* ws = " \t\n\r\f\v";

// trim from end of string (right)
void rtrim(std::string& s, const char* t = ws)
{
    s.erase(s.find_last_not_of(t) + 1);
}

// trim from beginning of string (left)
void ltrim(std::string& s, const char* t = ws)
{
    s.erase(0, s.find_first_not_of(t));
}

// trim from both ends of string (right then left)
void trim(std::string& s, const char* t = ws)
{
    ltrim(s, t);
    rtrim(s, t);
}