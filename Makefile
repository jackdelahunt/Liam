format:
	@find . -type f \( -name "*.h" -or -name "*.cpp" \) | xargs clang-format -i --style=file:".clang-format"

all:
	(cd liamc; vendor/premake/premake5 gmake; make config=debug)
	(cd liamc; ./bin/linux-x86_64/liamc/liamc --out="Code/out.cpp" --in="Code/main.liam" --time)
	(cd liamc; clang++ Code/out.cpp -I runtime -std=c++20 -o Code/a.out; ./Code/a.out)