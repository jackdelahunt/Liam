cmake_minimum_required(VERSION 3.22)
project(liamc)

set(CMAKE_CXX_STANDARD 20)

set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT "liamc" )

add_executable(liamc
        src/main.cpp
        src/args.cpp
        src/ast.cpp
        src/cpp_backend.cpp
        src/errors.cpp
        src/lexer.cpp
        src/parser.cpp
        src/token.cpp
        src/utils.cpp
        src/liam.cpp
        src/file.cpp
        src/type_checker.cpp
        src/compilation_unit.cpp
)

target_include_directories(liamc PUBLIC vendor)
