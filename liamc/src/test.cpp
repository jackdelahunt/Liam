#ifdef TEST
#include <cassert>
#include <iostream>

#include "file.h"

template <typename A, typename B> void assert_eq(A a, B b) {
    if (a != b)
    {
        std::cout << a << " != " << b << "\n";
        panic("assert_eq");
    }
}

void file_manager_index_at_test() {
    auto s                 = std::string("Hello world\nHello sailor\nAnother test string");
    FileManager::singleton = new FileManager();
    std::string path       = "temp";

    FileManager::singleton->files[path] = FileData{.data = std::vector<char>(s.begin(), s.end())};

    assert_eq(FileManager::load(&path)->index_at(1, 1), 0);
    assert_eq(FileManager::load(&path)->index_at(1, 11), 10);
    assert_eq(FileManager::load(&path)->index_at(2, 1), 12);
    assert_eq(FileManager::load(&path)->index_at(2, 12), 23);
}

void file_manager_line_test() {
    auto s                 = std::string("Hello world\nHello sailor\nAnother test string");
    FileManager::singleton = new FileManager();
    std::string path       = "temp";

    FileManager::singleton->files[path] = FileData{.data = std::vector<char>(s.begin(), s.end())};

    assert_eq(FileManager::load(&path)->line(1), "Hello world\n");
    assert_eq(FileManager::load(&path)->line(2), "Hello sailor\n");
    assert_eq(FileManager::load(&path)->line(3), "Another test string");
}

int main() {
    file_manager_index_at_test();
    file_manager_line_test();
}

#endif