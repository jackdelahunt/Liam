#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <chrono>
#include <vector>
#include "liam.h"
#include "lexer.h"
#include "parser.h"
#include "generator.h"
#include "emitter.h"
#include "C_backend/c_backend.h"
#include "type_checker.h"

int main(int argc, char** argv) {

#ifdef _DEBUG
    const char* source_path = "P:/Liam/main.liam";
    const char* out_path = "P:/Liam/build/main.cpp";
#else
    if (argc < 3) {
        panic("Not enough arguments");
    }

    char* source_path = argv[1];
    char* out_path = argv[2];
#endif
    
    auto lex_start = std::chrono::high_resolution_clock::now();
    auto lexer = Lexer();
    lexer.lex(source_path);
    auto lex_end = std::chrono::high_resolution_clock::now();
    auto lex_delta = lex_end - lex_start;
    
    auto parser_start = std::chrono::high_resolution_clock::now();
    auto parser = Parser(lexer.tokens);
    parser.parse();
    auto parse_end = std::chrono::high_resolution_clock::now();
    auto parser_delta = parse_end - parser_start;
    
    
    auto typing_start = std::chrono::high_resolution_clock::now();
    auto type_checker = TypeChecker();
    type_checker.type_file(&parser.root);
    auto typing_end = std::chrono::high_resolution_clock::now();
    auto typing_delta = typing_end - typing_start;
    
    auto code_generation_start = std::chrono::high_resolution_clock::now();
    auto c_backend = CBackend();
    auto c_code = c_backend.emit(&type_checker.root);
    auto code_generation_end = std::chrono::high_resolution_clock::now();
    auto code_generation_delta = code_generation_end - code_generation_start;
    
    std::cout << c_code;
    std::ofstream out_file(out_path);
    out_file << c_code;
    out_file.close();
    
    
    
    std::cout << "Lex time: " << duration_cast<std::chrono::milliseconds>(lex_delta) 
        << " " << duration_cast<std::chrono::microseconds>(lex_delta) << "\n";
    
    std::cout << "Parse time: " << duration_cast<std::chrono::milliseconds>(parser_delta) << " " 
        << duration_cast<std::chrono::microseconds>(parser_delta) << "\n";
    
    std:: cout << "Type check time: " << duration_cast<std::chrono::milliseconds>(typing_delta) << " " 
        << duration_cast<std::chrono::microseconds>(typing_delta) << "\n";

    std::cout << "Code generation time: " << duration_cast<std::chrono::milliseconds>(code_generation_delta) << " "
        << duration_cast<std::chrono::microseconds>(code_generation_delta) << "\n";
    
    return 0;
    
    //for (auto stmt : parser.root.statements) {
    //    std::cout << *stmt;
    //}
    
    
    /*auto emitter = Emitter();
    auto byte_code = emitter.emit(parser.root);
    std::cout << byte_code;

    std::ofstream out_file(out_path);
    out_file << byte_code;
    out_file.close();*/
    
    /*auto c_backend = CBackend();
    auto byte_code = c_backend.emit(parser.root);
    std::cout << byte_code;

    std::ofstream out_file(out_path);
    out_file << byte_code;
    out_file.close();*/
    
    
    // auto generator = liam::Generator();
    // auto [vm, err] = generator.generate("/home/jackdelahunt/Projects/Liam/main.l__m");
    // if(err) {
    //     printf("%s\n", err);
    // }
    // vm.run();
}