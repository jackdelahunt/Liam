#pragma once

#include <map>
#include <string>
#include <vector>

#include "liam.h"

struct FileData {
    char *data;
    u64 data_length;
    u64 line_count;

    u64 index_at(u32 line, u32 character);
    std::string line(u64 line);
};

struct FileManager {
    static FileManager *singleton;
    std::map<std::string, FileData> files;

    static FileData *load(std::string path);
    static std::map<std::string, FileData> *get_files();
};
