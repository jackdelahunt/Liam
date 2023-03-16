format:
	@find . -type f \( -name "*.h" -or -name "*.cpp" \) | xargs clang-format -i --style=file:".clang-format"

# -Dtarget=x86_64-windows
release:
	@zig build -Drelease-fast

build:
	@zig build -Drelease-safe

run: build
	@./zig-out/bin/liamc --stdlib="stdlib" --in="main.liam" -t
	@clang++ -I "stdlib/include"  -std=c++20 -o out.exe out.cpp
	@./out.exe
