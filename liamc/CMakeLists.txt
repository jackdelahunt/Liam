cmake_minimum_required(VERSION 3.20)
project(liamc)

set(CMAKE_CXX_STANDARD 20)

include_directories(liamc "include/")

add_executable(liamc
        "src/main.cpp"
        "src/expression.cpp" 
        "src/statement.cpp" 
        "src/lexer.cpp" 
        "src/parser.cpp" 
        "src/liam.cpp" 
        "src/emitter.cpp"
        "src/C_backend/c_backend.cpp"
        
        "include/liam.h"
        "include/generator.h"
        "include/lexer.h"
        "include/parser.h"
        "include/statement.h" 
        "include/expression.h" 
        "include/emitter.h"
        "include/C_backend/c_backend.h"
 "include/type_checker.h" "src/type_checker.cpp" )
