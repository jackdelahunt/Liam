#include <stdio.h>
#include <fstream>
#include <iostream>
#include <vector>
#include "liam.h"
#include "lexer.h"
#include "parser.h"
#include "generator.h"
#include "emitter.h"

int main() {
    auto lexer = Lexer();
    lexer.lex("E:/JacksDocuments/Projects/Liam/main.liam");
    /*for(auto& t: lexer.tokens) {
        std::cout << t << "\n";
    }*/

    auto parser = Parser(lexer.tokens);
    parser.parse();

    auto emitter = Emitter();

    std::cout << emitter.emit(parser.root);

    /*std::ofstream out("E:/JacksDocuments/Projects/Liam/main.l__m");
    out << emitter.emit(parser.root);
    out.close();*/


    // auto generator = liam::Generator();
    // auto [vm, err] = generator.generate("/home/jackdelahunt/Projects/Liam/main.l__m");
    // if(err) {
    //     printf("%s\n", err);
    // }
    // vm.run();
}