#include "file.h"
#include "liam.h"

#include <cassert>
#include <fstream>

FileManager *FileManager::singleton = NULL;

FileData *FileManager::load(std::filesystem::path path) {
    if (FileManager::singleton == NULL)
    { singleton = new FileManager(); }

    if (FileManager::singleton->files.count(path) > 0)
    { return &FileManager::singleton->files[path]; }

    u64 file_size_in_bytes = 0;

    {
        std::ifstream file_binary_read(path, std::ios::binary);
        file_binary_read.seekg(0, std::ios::end);
        file_size_in_bytes = file_binary_read.tellg();
    }

    std::ifstream file;
    file.open(path);
    ASSERT_MSG(file.is_open(), "All files should be possible to read as this is not user input");

    char *data = (char *)malloc(sizeof(char) * file_size_in_bytes);

    u64 line_count = 0;

    // this looks werid but it is for making sure we have the correct number
    // of lines counted. We check if we can get the first character, if it is
    // not EOF then we add it and we say there is a line count of 1. Then
    // we continue the process of loading the fiile. If we just counted all the
    // \n then we would miss the first line
    int first_character = file.get();
    if (first_character != EOF)
    {
        line_count++;
        //        vec->push_back((char)first_character);
        data[0] = (char)first_character;

        int index = 1;
        for (i32 i = file.get(); i != EOF; i = file.get())
        {
            char c = (char)i;
            if (c == '\n')
            { line_count++; }

            data[index] = c;
            index++;
        }
    }

    file.close();

    FileManager::singleton->files[path] =
        FileData{.path = path, .data = data, .data_length = file_size_in_bytes, .line_count = line_count};
    return &FileManager::singleton->files[path];
}

std::map<std::filesystem::path, FileData> *FileManager::get_files() {
    if (FileManager::singleton == NULL)
    { singleton = new FileManager(); }

    return &FileManager::singleton->files;
}
