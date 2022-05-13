cmake_minimum_required(VERSION 3.20)
project(liamc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER  /usr/bin/clang++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

include_directories(liamc "include/")

add_executable(liamc
        "src/main.cpp"
        "src/expression.cpp" 
        "src/statement.cpp" 
        "src/lexer.cpp" 
        "src/parser.cpp" 
        "src/liam.cpp"
        "src/rust_backend/rust_backend.cpp"
        "src/type_checker.cpp"

        "include/liam.h"
        "include/generator.h"
        "include/lexer.h"
        "include/parser.h"
        "include/statement.h"
        "include/expression.h"
        "include/rust_backend/rust_backend.h"
        "include/type_checker.h"
        "include/compiler.h"
)
