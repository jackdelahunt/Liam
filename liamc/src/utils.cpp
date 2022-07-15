#include "utils.h"

const char* ws = " \t\n\r\f\v";

// trim from end of string (right)
void rtrim(std::string& s, const char* t)
{
    s.erase(s.find_last_not_of(t) + 1);
}

// trim from beginning of string (left)
void ltrim(std::string& s, const char* t)
{
    s.erase(0, s.find_first_not_of(t));
}

// trim from both ends of string (right then left)
void trim(std::string& s, const char* t)
{
    ltrim(s, t);
    rtrim(s, t);
}

// trim from end of string (right)
std::string copy_rtrim(std::string s, const char* t)
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from beginning of string (left)
std::string copy_ltrim(std::string s, const char* t)
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from both ends of string (right then left)
std::string copy_trim(std::string s, const char* t)
{
    ltrim(s, t);
    rtrim(s, t);
    return s;
}