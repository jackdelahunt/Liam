#include <stdio.h>
#include <fstream>
#include <iostream>
#include <vector>
#include "liam.h"
#include "lexer.h"
#include "parser.h"
#include "generator.h"

int main() {
    auto lexer = liam::Lexer();
    lexer.lex("/home/jackdelahunt/Projects/Liam/main.liam");
    // for(auto& t: lexer.tokens) {
    //     std::cout << t;
    // }
    auto parser = liam::Parser(lexer.tokens);
    parser.parse();
    std::cout << *parser.root;


    // auto generator = liam::Generator();
    // auto [vm, err] = generator.generate("/home/jackdelahunt/Projects/Liam/main.l__m");
    // if(err) {
    //     printf("%s\n", err);
    // }
    // vm.run();
}