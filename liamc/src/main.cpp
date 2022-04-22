#include <stdio.h>
#include <fstream>
#include <iostream>
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
    char* out_path    = argv[2];
#endif



    auto lexer = Lexer();
    lexer.lex(source_path);
    /*for(auto& t: lexer.tokens) {
        std::cout << t << "\n";
    }*/

    auto parser = Parser(lexer.tokens);
    parser.parse();

    auto type_checker = TypeChecker();
    type_checker.type_file(&parser.root);
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