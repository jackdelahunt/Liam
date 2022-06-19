#include "liam.h"

#include <iostream>

void panic(const std::string &msg)
{
    std::cerr << "PANIC :: " << msg << std::endl;
    exit(69);
}