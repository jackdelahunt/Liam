#pragma once

#include "lexer.h"
#include "liam.h"
#include <string.h>
#include <string>
#include <tuple>

extern const char *ws;

bool is_digit(char c);
std::tuple<i64, NumberType, i32> extract_number_literal_size(std::string literal);
void rtrim(std::string &s, const char *t = ws);
void ltrim(std::string &s, const char *t = ws);
void trim(std::string &s, const char *t = ws);
std::string copy_rtrim(std::string s, const char *t = ws);
std::string copy_ltrim(std::string s, const char *t = ws);
std::string copy_trim(std::string s, const char *t = ws);

// used to compare against keywords below
// this showed a ~30% faster time then std::string::operator==
template <std::size_t N> bool compare_string(const std::string &s, char const (&literal)[N]) {
    return s.size() == N - 1 && memcmp(s.data(), literal, N - 1) == 0;
}

template <typename T> struct Option {
    explicit Option() {
        this->t     = {};
        this->valid = false;
    }

    explicit Option(T t) {
        this->t     = t;
        this->valid = true;
    }

    bool is_some() {
        return valid;
    }

    T value() {
        return this->t;
    }

  private:
    T t;
    bool valid;
};