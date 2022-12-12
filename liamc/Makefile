format:
	@find . -type f \( -name "*.h" -or -name "*.cpp" \) | xargs clang-format -i --style=file:".clang-format"

build:
	@./bin/liamc/liamc --stdlib="stdlib" --in="main.liam" -t

run: build
	@clang++ -I "stdlib/include"  -std=c++20 -o out.exe out.cpp
	@./out.exe