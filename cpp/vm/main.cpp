#include <stdio.h>
#include "liam.h"

int main() {
    auto arr = liam::Array<int>(1);
    arr.add(2);
    arr.add(3);
    arr.add(2);
    arr.add(4);
    arr.print();
}