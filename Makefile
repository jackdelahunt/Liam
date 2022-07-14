format:
	@find . -type f \( -name "*.h" -or -name "*.cpp" \) | xargs clang-format -i --style=file:".clang-format"

gmake:
	(cd liamc; vendor/premake/premake5 gmake)

gmake-build: gmake
	(cd liamc; make config=debug)

gmake-test: gmake
	(cd liamc; make config=test)

cmake:
	(cd liamc; vendor/premake/premake5 cmake)

vs:
	(cd liamc; vendor/premake/premake5 vs2022)

run:
	./liamc/bin/linux-x86_64/liamc/liamc --out="liamc/Code/out.cpp" --in="liamc/Code/main.liam" --time
	clang++ liamc/Code/out.cpp -I liamc/runtime -std=c++20 -o liamc/Code/a.out; 
	./liamc/Code/a.out