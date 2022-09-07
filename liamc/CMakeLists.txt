cmake_minimum_required(VERSION 3.22)
project(liamc)

# must specify PATH_TO_LLVM and LLVM_LIBS to cmake
message("Using path of llvm -> " ${PATH_TO_LLVM})
message("Using llvm libs -> " ${LLVM_LIBS})

set (search_paths
        ${PATH_TO_LLVM}
        ${PATH_TO_LLVM}/lib/cmake
        ${PATH_TO_LLVM}/lib/cmake/llvm
        ${PATH_TO_LLVM}/lib/cmake/clang
        ${PATH_TO_LLVM}/share/clang/cmake/
        ${PATH_TO_LLVM}/share/llvm/cmake/
        )

message(${search_paths})

find_package(LLVM REQUIRED CONFIG
        PATHS ${search_paths}
        NO_DEFAULT_PATH)

set(CMAKE_CXX_STANDARD 20)

include_directories(${PATH_TO_LLVM}/include)
link_directories(${PATH_TO_LLVM}/lib)

add_executable(liamc
        src/fmt/color.h
        src/fmt/core.h
        src/fmt/format.cc
        src/fmt/format.h
        src/fmt/format-inl.h
        src/args.cpp
        src/args.h
        src/compiler.h
        src/cpp_backend.cpp
        src/cpp_backend.h
        src/cxxopts.h
        src/errors.cpp
        src/errors.h
        src/expression.cpp
        src/expression.h
        src/file.cpp
        src/file.h
        src/lexer.cpp
        src/lexer.h
        src/liam.cpp
        src/liam.h
        src/main.cpp
        src/parser.cpp
        src/parser.h
        src/statement.cpp
        src/statement.h
        src/test.cpp
        src/type_checker.cpp
        src/type_checker.h
        src/type_info.h
        src/utils.cpp
        src/utils.h
        src/llvm_backend.h
        src/llvm_backend.cpp
        )

target_link_libraries(liamc ${LLVM_LIBS} z)