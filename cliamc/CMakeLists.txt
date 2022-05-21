cmake_minimum_required(VERSION 3.22)
project(cliamc C)

set(CMAKE_C_STANDARD 17)

add_executable(cliamc
        main.c
        liam.c

        lexer.h
        parser.h
        str.h
        arena.h
        macros.h
        parser.h
        )

# math library
target_link_libraries(cliamc m)
