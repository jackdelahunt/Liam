format:
	@find . -type f \( -name "*.h" -or -name "*.cpp" \) | xargs clang-format -i --style=file:".clang-format"

run:
	./liamc/bin/liamc/liamc --out="liamc/Code/out.cpp" --in="liamc/Code/main.liam" --time
	clang++ liamc/Code/out.cpp -I liamc/runtime -std=c++20 -o liamc/Code/a.out; 
	./liamc/Code/a.out