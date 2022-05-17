#include "lexer.h"
#include "arena.h"

int main(void) {

    Arena arena = make_arena(50 * 1024 * 1024); // 50MB
    current__working__arena__ = &arena;

    Lexer lexer = make_lexer();
    lex(&lexer, "/home/jackdelahunt/Projects/Liam/cliamc/main.liam");

    for(int i = 0; i < lexer.count; i++) {
        printf("Token :: id -> %d, length -> %lu, string -> ", lexer.tokens[i].type, lexer.tokens[i].slice.length);
        print_slice(&lexer.tokens[i].slice);
    }

    free_arena(&arena);
    return 0;
}

