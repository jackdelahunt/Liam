#include <stdio.h>
#include <fstream>
#include <iostream>
#include <vector>
#include "liam.h"
#include "lexer.h"
#include "parser.h"
#include "generator.h"

int main() {
    auto lexer = Lexer();
    lexer.lex("E:/JacksDocuments/Projects/Liam/main.liam");
    /*for(auto& t: lexer.tokens) {
        std::cout << t << "\n";
    }*/

    auto parser = Parser(lexer.tokens);
    parser.parse();
    for (auto stmt : parser.root.statements) {
        std::cout << *stmt << "\n";
    }


    // auto generator = liam::Generator();
    // auto [vm, err] = generator.generate("/home/jackdelahunt/Projects/Liam/main.l__m");
    // if(err) {
    //     printf("%s\n", err);
    // }
    // vm.run();
}