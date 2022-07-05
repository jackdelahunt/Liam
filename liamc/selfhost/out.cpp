#include "lib.h"

// struct forward declarations

// function forward declarations
void __liam__main__();

// Source
void __liam__main__() {
    auto path = make_str((char *)"/home/jackdelahunt/Projects/Liam/liamc/selfhost/test.liam", 57);
    auto source = read(path);
    for (auto index = make_u64(0); index < string_length(&(source)); index = index + make_u64(1))
    {
        auto current_char = string_substring(&(source), index, make_u64(1));
        if (current_char == make_str((char *)"h", 1))
        { println<str>(make_str((char *)"its h", 5)); }
    }
}

int main(int argc, char **argv) {
    __liam__main__();
}
