#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <filesystem>
#include "liam.h"
#include "lexer.h"
#include "parser.h"
#include "rust_backend/rust_backend.h"
#include "type_checker.h"

int main(int argc, char** argv) {

    if (argc < 2) {
        panic("Not enough arguments");
    }
    std::filesystem::path source = argv[1];

    auto parse_lex_start = std::chrono::high_resolution_clock::now();
    auto lexer = Lexer(absolute(source).string());
    lexer.lex();

    auto parser = Parser(absolute(source).string(), lexer.tokens);
    parser.parse();
    if(parser.errors.size() > 0) {
        for(auto& err : parser.errors) {
            std::cout << parser.path << ":" << err.line << ":" << err.character << " :: " << err.error << "\n";
        }
        return EXIT_FAILURE;
    }

    auto parse_lex_end= std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> parse_lex_delta= parse_lex_end- parse_lex_start;

    auto typing_start = std::chrono::high_resolution_clock::now();
    auto type_checker = TypeChecker();
    type_checker.type_file(&parser.root);
    auto typing_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> typing_delta = typing_end - typing_start;

    auto code_generation_start = std::chrono::high_resolution_clock::now();
    auto rust_backend = RustBackend();
    auto code = rust_backend.emit(type_checker.root);
    auto code_generation_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> code_generation_delta = code_generation_end - code_generation_start;

    std::ofstream out_file("main.rs");
    out_file << code;
    out_file.close();

    std::cout << "Parse & Lex time: " << parse_lex_delta.count() << "ms\n";
    std::cout << "Type check time: " << typing_delta.count() << "ms\n";
    std::cout << "Code generation time: " << code_generation_delta.count() << "ms\n";

    system(std::string("rustc " + std::string("main.rs")).c_str());
    // system(std::string("rustfmt " + std::string("main.rs")).c_str());

    return 0;
}