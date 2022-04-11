#include <stdio.h>
#include "liam.h"

int main() {
    auto vm = liam::VM{};
    vm.push_byte(liam::OpCode::PUSH);
    vm.push_byte(10);
    vm.push_byte(liam::OpCode::PRINT);
    vm.run();
}