#ifdef TEST
#include <iostream>

#include "file.h"
#include "utils.h"

#define ASSERT_EQ(a, b)                                                                                                \
    {                                                                                                                  \
        auto A = a;                                                                                                    \
        auto B = b;                                                                                                    \
        if (A != B)                                                                                                    \
        {                                                                                                              \
            std::cout << #a << " != " << #b << "\n";                                                                   \
            std::cout << "Evaluated ::" << A << " != " << B << "\n";                                                   \
            panic("Assertion error");                                                                                  \
        }                                                                                                              \
    }

void file_manager_index_at_test() {
    auto s                 = std::string("Hello world\nHello sailor\nAnother test string");
    FileManager::singleton = new FileManager();
    std::string path       = "temp";

    FileManager::singleton->files[path] = FileData{.data = std::vector<char>(s.begin(), s.end())};

    ASSERT_EQ(FileManager::load(&path)->index_at(1, 1), 0);
    ASSERT_EQ(FileManager::load(&path)->index_at(1, 11), 10);
    ASSERT_EQ(FileManager::load(&path)->index_at(2, 1), 12);
    ASSERT_EQ(FileManager::load(&path)->index_at(2, 12), 23);
}

void file_manager_line_test_1() {
    auto s                 = std::string("Hello world\nHello sailor\nAnother test string");
    FileManager::singleton = new FileManager();
    std::string path       = "temp";

    FileManager::singleton->files[path] = FileData{.data = std::vector<char>(s.begin(), s.end())};

    ASSERT_EQ(FileManager::load(&path)->line(1), "Hello world\n");
    ASSERT_EQ(FileManager::load(&path)->line(2), "Hello sailor\n");
    ASSERT_EQ(FileManager::load(&path)->line(3), "Another test string");
}

void file_manager_line_test_2() {
    std::string s          = R"(fn main(): void {
    let x :=
    "";

    let y: u64 = x;
}
)";
    FileManager::singleton = new FileManager();
    std::string path       = "temp";

    FileManager::singleton->files[path] = FileData{.data = std::vector<char>(s.begin(), s.end())};

    ASSERT_EQ(FileManager::load(&path)->line(1), "fn main(): void {\n");
    ASSERT_EQ(FileManager::load(&path)->line(2), "    let x :=\n");
    ASSERT_EQ(FileManager::load(&path)->line(3), "    \"\";\n");
    ASSERT_EQ(FileManager::load(&path)->line(4), "\n");
    ASSERT_EQ(FileManager::load(&path)->line(5), "    let y: u64 = x;\n");
    ASSERT_EQ(FileManager::load(&path)->line(6), "}\n");
}

void extract_number_literal_size_test() {

    // error types
    {
        auto [literal, type, size] = extract_number_literal_size("--..0s32");
        ASSERT_EQ(literal, 0);
        ASSERT_EQ(type, UNSIGNED);
        ASSERT_EQ(size, -1);
    }

    // negative and floating points
    {
        auto [literal, type, size] = extract_number_literal_size("-124s32");
        ASSERT_EQ(literal, -124);
        ASSERT_EQ(type, SIGNED);
        ASSERT_EQ(size, 32);
    }

    //    {
    //        auto [literal, type, size] = extract_number_literal_size("-12.2344s64");
    //        ASSERT_EQ(literal, -12.2344);
    //        ASSERT_EQ(type, SIGNED);
    //        ASSERT_EQ(size, 64);
    //    }

    // all basic types, all positive and non-floating point
    {
        auto [literal, type, size] = extract_number_literal_size("12u8");
        ASSERT_EQ(literal, 12);
        ASSERT_EQ(type, UNSIGNED);
        ASSERT_EQ(size, 8);
    }

    {
        auto [literal, type, size] = extract_number_literal_size("16s8");
        ASSERT_EQ(literal, 16);
        ASSERT_EQ(type, SIGNED);
        ASSERT_EQ(size, 8);
    }

    {
        auto [literal, type, size] = extract_number_literal_size("20u16");
        ASSERT_EQ(literal, 20);
        ASSERT_EQ(type, UNSIGNED);
        ASSERT_EQ(size, 16);
    }

    {
        auto [literal, type, size] = extract_number_literal_size("24s16");
        ASSERT_EQ(literal, 24);
        ASSERT_EQ(type, SIGNED);
        ASSERT_EQ(size, 16);
    }

    {
        auto [literal, type, size] = extract_number_literal_size("28u32");
        ASSERT_EQ(literal, 28);
        ASSERT_EQ(type, UNSIGNED);
        ASSERT_EQ(size, 32);
    }

    {
        auto [literal, type, size] = extract_number_literal_size("32s32");
        ASSERT_EQ(literal, 32);
        ASSERT_EQ(type, SIGNED);
        ASSERT_EQ(size, 32);
    }

    {
        auto [literal, type, size] = extract_number_literal_size("36f32");
        ASSERT_EQ(literal, 36);
        ASSERT_EQ(type, FLOAT);
        ASSERT_EQ(size, 32);
    }

    {
        auto [literal, type, size] = extract_number_literal_size("40u64");
        ASSERT_EQ(literal, 40);
        ASSERT_EQ(type, UNSIGNED);
        ASSERT_EQ(size, 64);
    }

    {
        auto [literal, type, size] = extract_number_literal_size("44s64");
        ASSERT_EQ(literal, 44);
        ASSERT_EQ(type, SIGNED);
        ASSERT_EQ(size, 64);
    }

    {
        auto [literal, type, size] = extract_number_literal_size("48f64");
        ASSERT_EQ(literal, 48);
        ASSERT_EQ(type, FLOAT);
        ASSERT_EQ(size, 64);
    }
}

int main() {
    file_manager_index_at_test();
    file_manager_line_test_1();
    file_manager_line_test_2();
    extract_number_literal_size_test();
}

#endif