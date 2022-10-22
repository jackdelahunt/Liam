#include "file.h"

#include <cassert>
#include <fstream>

u64 FileData::index_at(u32 line, u32 character) {
    character--;
    line--;

    u64 current_line = 0;
    u64 index        = 0;

    while (current_line != line)
    {
        if (data.at(index) == '\n')
            current_line++;

        index++;
    }

    assert(index + character < data.size());
    return index + character;
}

std::string FileData::line(u64 line) {

    assert(line > 0);

    auto start = index_at(line, 1);
    auto end   = start;
    std::string s;

    s.push_back(data.at(end));
    end++;

    if (s != "\n")
    {
        while (true)
        {
            if (end >= data.size())
                break;
            if (data.at(end) == '\n')
                break;

            s.push_back(data.at(end));
            end++;
        }
        // add new line
        if (end < data.size())
            s.push_back(data.at(end));
    }

    return s;
}

FileManager *FileManager::singleton = NULL;

FileData *FileManager::load(std::string *path) {
    if (FileManager::singleton == NULL)
    { singleton = new FileManager(); }

    if (FileManager::singleton->files.count(*path) > 0)
    { return &FileManager::singleton->files[*path]; }

    auto vec = std::vector<char>();

    std::ifstream file;
    file.open(*path);
    if (!file.is_open())
    { panic("cannot open file " + *path); }

    u64 line_count = 0;
    for (s32 i = file.get(); i != EOF; i = file.get())
    {
        if ((char)i == '\n')
        { line_count++; }

        vec.push_back((char)i);
    }

    file.close();

    FileManager::singleton->files[*path] = FileData{.data = vec, .line_count = line_count};
    return &FileManager::singleton->files[*path];
}
