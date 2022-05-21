#include "lexer.h"
#include "parser.h"
#include "arena.h"

int main(void) {

    Arena arena = make_arena(50 * 1024 * 1024); // 50MB
    current__working__arena__ = &arena;

    Lexer lexer = make_lexer();
    lex(&lexer, "/home/jackdelahunt/Projects/Liam/cliamc/main.liam");

    Parser parser = make_parser(&lexer);
    print_ast(&parser);

    free_arena(&arena);
    return 0;
}

