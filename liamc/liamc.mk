release:
	./vendor/premake5-mac gmake
	mkdir release

	mkdir release/win_64
	make config=release_win64
	mv bin/liamc/liamc release/win_64

	mkdir release/macos
	make config=release_macos
	mv bin/liamc/liamc release/macos

	mkdir release/linux_64
	make config=release_linux64
	mv bin/liamc/liamc release/linux_64

	cp -r stdlib release/win_64
	cp -r stdlib release/macos
	cp -r stdlib release/linux_64

format:
	@find . -type f \( -name "*.h" -or -name "*.cpp" \) | xargs clang-format -i --style=file:".clang-format"

build:
	@./bin/liamc/liamc --stdlib="stdlib" --in="main.liam" -t

run: build
	@clang++ -I "stdlib/include"  -std=c++20 -o out.exe out.cpp
	@./out.exe

