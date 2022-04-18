#pragma once
#include <string.h>
#include <stdlib.h>

struct string {
  char* str;
  int size;

  string(const char* c_str) {
    size = strlen(c_str);
    str = (char*)malloc(sizeof(char) * size);
    memcpy(str, c_str, size);
  }

  string  operator+(string const &other) {
    string s = "new string";
    return s;
  }

  ~string() {
    free(str);
  }
};
