#pragma once

#include <vector>
#include <map>
#include <string>

#include "liam.h"

struct FileData {
    std::vector<char> data;
    u64 line_count;

    u64 index_at(u32 line, u32 character);
    std::string line(u64 line);
};

struct FileManager {
    static FileManager *singleton;
    std::map<std::string, FileData> files;

    static FileData *load(std::string *path);
};
