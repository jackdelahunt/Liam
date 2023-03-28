#include "file.h"
#include "liam.h"

#include <cassert>
#include <fstream>

u64 FileData::index_at(u32 line, u32 character) {

    ASSERT(line > 0);
    ASSERT(character > 0);

    u32 current_line = 1;
    u32 current_char = 1;
    for (i64 i = 0; i < this->data_length; i++)
    {

        if (current_line == line && current_char == character)
        { return i; }

        if (this->data[i] == '\n')
        {
            current_line++;
            current_char = 1;
        }
        else
        { current_char++; }
    }

    ASSERT_MSG(0, "Could not find line and character in file data");
}

std::string FileData::line(u64 line) {

    ASSERT(line > 0);
    ASSERT(line <= this->line_count);
    ASSERT(this->data);

    auto start = index_at(line, 1);
    auto end   = start;
    std::string s;

    s.push_back(this->data[end]);
    end++;

    if (s != "\n")
    {
        while (true)
        {
            if (end >= this->data_length)
                break;
            if (this->data[end] == '\n')
                break;

            s.push_back(this->data[end]);
            end++;
        }
        // add new line
        if (end < this->data_length)
            s.push_back(this->data[end]);
    }

    return s;
}

FileManager *FileManager::singleton = NULL;

FileData *FileManager::load(std::string *path) {
    if (FileManager::singleton == NULL)
    { singleton = new FileManager(); }

    if (FileManager::singleton->files.count(*path) > 0)
    { return &FileManager::singleton->files[*path]; }

    u64 file_size_in_bytes = 0;

    {
        std::ifstream file_binary_read(*path, std::ios::binary);
        file_binary_read.seekg(0, std::ios::end);
        file_size_in_bytes = file_binary_read.tellg();
    }

    std::ifstream file;
    file.open(*path);
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

    FileManager::singleton->files[*path] = FileData{.data = data, .data_length = file_size_in_bytes, .line_count = line_count};
    return &FileManager::singleton->files[*path];
}
