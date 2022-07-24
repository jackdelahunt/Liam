#include "lib.h"

// struct forward declarations

// function forward declarations
void __liam__main__();

// Source
void __liam__main__() {
    auto x = _s64(100);
    auto y = _s64(200);
    println<s64>(x + y);
}

int main(int argc, char **argv) {
    __liam__main__();
}