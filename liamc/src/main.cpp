#include <stdio.h>
#include <fstream>
#include <iostream>
#include <vector>
#include "liam.h"
#include "lexer.h"
#include "parser.h"
#include "generator.h"
#include "emitter.h"

int main(int argc, char** argv) {

#ifdef _DEBUG
    const char* source_path = "P:/Liam/main.liam";
    const char* out_path = "P:/Liam/main.l__m";
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

    //for (auto stmt : parser.root.statements) {
    //    std::cout << *stmt;
    //}

    
    auto emitter = Emitter();
    auto byte_code = emitter.emit(parser.root);
    std::cout << byte_code;

    std::ofstream out_file(out_path);
    out_file << byte_code;
    out_file.close();


    // auto generator = liam::Generator();
    // auto [vm, err] = generator.generate("/home/jackdelahunt/Projects/Liam/main.l__m");
    // if(err) {
    //     printf("%s\n", err);
    // }
    // vm.run();
}