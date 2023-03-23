format:
	@find . -type f \( -name "*.h" -or -name "*.cpp" \) | xargs clang-format -i --style=file:".clang-format"

# -Dtarget=x86_64-windows
release:
	@zig build -Drelease-fast

build:
	@zig build -Drelease-safe

run: build
	@./zig-out/bin/liamc --stdlib="stdlib" --in="program/main.liam" -t
	@clang++ -I "stdlib/include"  -std=c++20 -o out.exe out.cpp
	@./out.exe

clean:
	@rm -rf zig-cache
	@rm -rf zig-out
	@rm -f out.cpp
	@rm -f out.exe
	@rm -rf .idea
	@rm -rf cmake-build-debug
