cmake_minimum_required(VERSION 3.20)
project(liamvm)

set(CMAKE_CXX_STANDARD 20)

include_directories(liamvm "include/")

add_executable(liamvm
        "src/main.cpp"
        
        "include/liam.h"
        "include/generator.h"
        "include/lexer.h"
        "include/parser.h"
        "include/statement.h" "include/expression.h" "src/expression.cpp" "src/statement.cpp" "src/lexer.cpp" "src/parser.cpp" "src/liam.cpp" "include/emitter.h" "src/emitter.cpp")
