#include "file.h"
#include "liam.h"
#include "utils.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <vector>

FileManager *FileManager::singleton = NULL;

Option<FileData *> FileManager::load_relative_from_cwd(std::string path) {
    if (FileManager::singleton == NULL) {
        singleton = new FileManager();
    }

    return singleton->load_relative_to(std::filesystem::current_path().string(), path);
}

Option<FileData *> FileManager::load_relative_to(std::string relative_to, std::string path) {
    if (FileManager::singleton == NULL) {
        singleton = new FileManager();
    }

    std::filesystem::path relative_slash_path = std::filesystem::path(relative_to) / path;
    std::filesystem::path absolute_path       = std::filesystem::weakly_canonical(relative_slash_path);

    if (!std::filesystem::exists(absolute_path)) {
        return Option<FileData *>();
    }

    for (u64 i = 0; i < singleton->files.size(); i++) {

        if (singleton->files[i]->absolute_path == absolute_path) {
            return Option(singleton->files[i]);
        }
    }

    u64 file_size_in_bytes = 0;

    {
        std::ifstream file_binary_read(absolute_path.string(), std::ios::binary);
        file_binary_read.seekg(0, std::ios::end);
        file_size_in_bytes = file_binary_read.tellg();
    }

    std::ifstream file;
    file.open(absolute_path.string());
    char *data          = (char *)malloc(sizeof(char) * file_size_in_bytes);

    u64 line_count      = 0;

    // this looks werid but it is for making sure we have the correct number
    // of lines counted. We check if we can get the first character, if it is
    // not EOF then we add it and we say there is a line count of 1. Then
    // we continue the process of loading the file. If we just counted all the
    // \n then we would miss the first line
    int first_character = file.get();
    if (first_character != EOF) {
        line_count++;
        //        vec->push_back((char)first_character);
        data[0]   = (char)first_character;

        int index = 1;
        for (i32 i = file.get(); i != EOF; i = file.get()) {
            char c = (char)i;
            if (c == '\n') {
                line_count++;
            }

            data[index] = c;
            index++;
        }
    }

    file.close();

    FileManager::singleton->files.push_back(new FileData{
        .absolute_path = absolute_path, .data = data, .data_length = file_size_in_bytes, .line_count = line_count});
    return Option(singleton->files.back());
}

std::vector<FileData *> *FileManager::get_files() {
    if (FileManager::singleton == NULL) {
        singleton = new FileManager();
    }

    return &singleton->files;
}

FileManager::FileManager() {
    this->files = std::vector<FileData *>();
}
