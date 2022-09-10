format:
	@find . -type f \( -name "*.h" -or -name "*.cpp" \) | xargs clang-format -i --style=file:".clang-format"

run:
	./liamc/build-debug/liamc --cpp --emit --cpp-source="liamc/Code/out.cpp" --include=liamc/runtime --stdlib=liamc/stdlib --out="liamc/Code/out.exe" --in="liamc/Code/main.liam" --time --debug 
	./liamc/Code/out.exe

run-test:
	./liamc/build-debug/liamc --cpp --test --emit --cpp-source="liamc/Code/out.cpp" --include=liamc/runtime --stdlib=liamc/stdlib --out="liamc/Code/ou
	./liamc/Code/out.exe

selfhost:
	./liamc/build-debug/liamc --cpp --cpp-source="liamc/build-debug/out.cpp" --include=liamc/runtime --stdlib=liamc/stdlib --out="liamc/build-debug/stage2.exe" --in="liamc/selfhost/main.liam" --time --debug
	
	@clang-format -i liamc/build-debug/out.cpp
	./liamc/build-debug/stage2.exe
