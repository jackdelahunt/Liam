format:
	@find . -type f \( -name "*.h" -or -name "*.cpp" \) | xargs clang-format -i --style=file:".clang-format"

# -Dtarget=x86_64-windows
release:
	@zig build -Drelease-fast

build:
	@zig build -Drelease-safe

run: build
	@./zig-out/bin/liamc --stdlib="stdlib" --in="program/main.liam" -t
	@clang++ -I "stdlib/include" -I core -std=c++20 -o out.exe out.cpp
	@clang-format -i out.cpp
	@./out.exe

manual:
	clang++ -std=c++20 src/main.cpp src/args.cpp src/module.cpp src/compiler.cpp src/cpp_backend.cpp \
	src/errors.cpp src/expression.cpp src/file.cpp src/lexer.cpp src/liam.cpp src/parser.cpp \
	src/statement.cpp src/type_checker.cpp src/type_info.cpp src/utils.cpp src/fmt/format.cc \
	-o zig-out/bin/liamc -g

clean:
	@rm -rf zig-cache
	@rm -rf zig-out
	@rm -f out.cpp
	@rm -f out.exe
	@rm -rf .idea
	@rm -rf cmake-build-debug
