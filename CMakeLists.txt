cmake_minimum_required(VERSION 3.20)
project(liamvm)

set(CMAKE_CXX_STANDARD 20)

include_directories(liamvm "include/")

add_executable(liamvm
        src/main.cpp
        
        include/liam.h
        include/generator.h
)
