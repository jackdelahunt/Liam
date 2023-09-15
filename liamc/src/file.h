#pragma once

#include <map>
#include <string>
#include <vector>
#include <filesystem>

#include "liam.h"

struct FileData {
    std::filesystem::path path;
    char *data;
    u64 data_length;
    u64 line_count;

    u64 index_at(u32 line, u32 character);
    std::string line(u64 line);
};

struct FileManager {
    static FileManager *singleton;
    std::map<std::filesystem::path, FileData> files;

    static FileData *load(std::filesystem::path path);
    static std::map<std::filesystem::path, FileData> *get_files();
};
