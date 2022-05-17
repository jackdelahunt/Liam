cmake_minimum_required(VERSION 3.22)
project(cliamc C)

set(CMAKE_C_STANDARD 17)

add_executable(cliamc
        main.c

        lexer.h
        str.h
        arena.h
        macros.h
        )
