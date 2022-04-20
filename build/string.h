
#pragma once
#include <string.h>
#include <stdlib.h>

struct string {
  char* str;

  string(const char* c_str) {
    int size = strlen(c_str);
    str = (char*)malloc(sizeof(char) * size);
    memcpy(str, c_str, size);
  }

  ~string() {
    free(str);
  }
};
