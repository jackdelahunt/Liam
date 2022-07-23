#ifdef TEST
#include <iostream>

#include "file.h"
#include "utils.h"

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

void extract_number_literal_size_test() {

    // error types
    {
        auto [literal, type, size] = extract_number_literal_size("--..0s32");
        assert_eq(literal, 0);
        assert_eq(type, UNSIGNED);
        assert_eq(size, -1);
    }

    // negative and floating points
    {
        auto [literal, type, size] = extract_number_literal_size("-124s32");
        assert_eq(literal, -124);
        assert_eq(type, SIGNED);
        assert_eq(size, 32);
    }

    {
        auto [literal, type, size] = extract_number_literal_size("-12.2344s64");
        assert_eq(literal, -12.2344);
        assert_eq(type, SIGNED);
        assert_eq(size, 64);
    }

    // all basic types, all positive and non-floating point
    {
        auto [literal, type, size] = extract_number_literal_size("12u8");
        assert_eq(literal, 12);
        assert_eq(type, UNSIGNED);
        assert_eq(size, 8);
    }

    {
        auto [literal, type, size] = extract_number_literal_size("16s8");
        assert_eq(literal, 16);
        assert_eq(type, SIGNED);
        assert_eq(size, 8);
    }

    {
        auto [literal, type, size] = extract_number_literal_size("20u16");
        assert_eq(literal, 20);
        assert_eq(type, UNSIGNED);
        assert_eq(size, 16);
    }

    {
        auto [literal, type, size] = extract_number_literal_size("24s16");
        assert_eq(literal, 24);
        assert_eq(type, SIGNED);
        assert_eq(size, 16);
    }

    {
        auto [literal, type, size] = extract_number_literal_size("28u32");
        assert_eq(literal, 28);
        assert_eq(type, UNSIGNED);
        assert_eq(size, 32);
    }

    {
        auto [literal, type, size] = extract_number_literal_size("32s32");
        assert_eq(literal, 32);
        assert_eq(type, SIGNED);
        assert_eq(size, 32);
    }

    {
        auto [literal, type, size] = extract_number_literal_size("36f32");
        assert_eq(literal, 36);
        assert_eq(type, FLOAT);
        assert_eq(size, 32);
    }

    {
        auto [literal, type, size] = extract_number_literal_size("40u64");
        assert_eq(literal, 40);
        assert_eq(type, UNSIGNED);
        assert_eq(size, 64);
    }

    {
        auto [literal, type, size] = extract_number_literal_size("44s64");
        assert_eq(literal, 44);
        assert_eq(type, SIGNED);
        assert_eq(size, 64);
    }

    {
        auto [literal, type, size] = extract_number_literal_size("48f64");
        assert_eq(literal, 48);
        assert_eq(type, FLOAT);
        assert_eq(size, 64);
    }
}

int main() {
    file_manager_index_at_test();
    file_manager_line_test();
    extract_number_literal_size_test();
}

#endif