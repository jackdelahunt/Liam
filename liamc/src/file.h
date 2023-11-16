#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "baseLayer/types.h"
#include "liam.h"
#include "utils.h"

struct FileData {
    std::filesystem::path absolute_path;
    char *data;
    u64 data_length;
    u64 line_count;
};

struct FileManager {
    static FileManager *singleton;
    // heap allocated because then we can append to this
    // list without worry of the ponters being invalidated
    std::vector<FileData *> files;

    static Option<FileData *> load_relative_from_cwd(std::string path);
    static Option<FileData *> load_relative_to(std::string relative_to, std::string path);
    static std::vector<FileData *> *get_files();

  private:
    FileManager();
};
