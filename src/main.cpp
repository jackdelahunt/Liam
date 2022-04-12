#include <stdio.h>
#include <fstream>
#include <iostream>
#include <vector>
#include "liam.h"
#include "generator.h"

int main() {
    auto generator = liam::Generator();
    auto [vm, err] = generator.generate("/home/jackdelahunt/Projects/Liam/main.l__m");
    if(err) {
        printf("%s\n", err);
    }
    vm.run();
}