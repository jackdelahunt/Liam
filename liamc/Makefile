format:
	@find . -type f \( -name "*.h" -or -name "*.cpp" \) | xargs clang-format -i --style=file:".clang-format"

run:
	./bin/liamc/liamc --in=main.liam --stdlib="stdlib" --out=bin/out.cpp
	@clang++ -I "stdlib/include" -g -O0 -I core -std=c++20 -o "bin/out.exe" bin/out.cpp
#	@clang-format -i program/out.cpp
	@./bin/out.exe

manual:
	clang++ -std=c++20 src/main.cpp src/args.cpp src/module.cpp src/compiler.cpp src/cpp_backend.cpp \
	src/errors.cpp src/ast.cpp src/file.cpp src/lexer.cpp src/liam.cpp src/parser.cpp \
	src/type_checker.cpp src/type_info.cpp src/utils.cpp src/fmt/format.cc \
	-o zig-out/bin/liamc -g

clean:
	@rm -rf zig-cache
	@rm -rf zig-out
	@rm -f out.cpp
	@rm -f out.exe
	@rm -rf .idea
	@rm -rf cmake-build-debug
