#include "utils.h"

const char *ws = " \t\n\r\f\v";

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

std::tuple<s64, NumberType, s32> extract_number_literal_size(std::string literal) {

#define BAD_PARSE                                                                                                      \
    { 0, UNSIGNED, -1 }

    int literal_end = 0;
    while (literal_end < literal.size() &&
           (is_digit(literal.at(literal_end)) || literal.at(literal_end) == '-' || literal.at(literal_end) == '.'))
    { literal_end++; }

    // literal == 0..literal_end
    // type == literal_end..literal_end +1
    // postfix == literal_end + 1..string_end

    auto literal_string = literal.substr(0, literal_end);

    std::string type_string    = "";
    std::string postfix_string = "";

    // if there is a postfix notation
    // else just infer a s64
    if (literal_end != literal.size())
    {
        type_string    = literal.substr(literal_end, 1);
        postfix_string = literal.substr(literal_end + 1, literal.size() - literal_end);
    }
    else
    {
        auto n = std::stod(literal_string);
        return {n, SIGNED, 64};
    }

    int size;

    try
    { size = std::stoi(postfix_string); }
    catch (std::exception &e)
    { return BAD_PARSE; }

    NumberType type;

    if (type_string == "u")
    { type = UNSIGNED; }
    else if (type_string == "f")
    { type = FLOAT; }
    else if (type_string == "s")
    { type = SIGNED; }
    else
    { return BAD_PARSE; }

    if (size == 8 || size == 16 || size == 32 || size == 64)
    {
        try
        {
            auto n = std::stod(literal_string);
            return {n, type, size};
        }
        catch (std::exception &e)
        {}
    }

    return BAD_PARSE;
}

// trim from end of string (right)
void rtrim(std::string &s, const char *t) {
    s.erase(s.find_last_not_of(t) + 1);
}

// trim from beginning of string (left)
void ltrim(std::string &s, const char *t) {
    s.erase(0, s.find_first_not_of(t));
}

// trim from both ends of string (right then left)
void trim(std::string &s, const char *t) {
    ltrim(s, t);
    rtrim(s, t);
}

// trim from end of string (right)
std::string copy_rtrim(std::string s, const char *t) {
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from beginning of string (left)
std::string copy_ltrim(std::string s, const char *t) {
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from both ends of string (right then left)
std::string copy_trim(std::string s, const char *t) {
    ltrim(s, t);
    rtrim(s, t);
    return s;
}