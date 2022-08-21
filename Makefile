format:
	@find . -type f \( -name "*.h" -or -name "*.cpp" \) | xargs clang-format -i --style=file:".clang-format"

run:
	./liamc/bin/liamc/liamc --codegen --emit="liamc/Code/out.cpp" --include=liamc/runtime --stdlib=liamc/stdlib --out="liamc/Code/out.exe" --in="liamc/Code/main.liam" --time --debug 
	./liamc/Code/out.exe

run-test:
	./liamc/bin/liamc/liamc --test --codegen --emit="liamc/Code/out.cpp" --include=liamc/runtime --stdlib=liamc/stdlib --out="liamc/Code/ou
	./liamc/Code/out.exe

selfhost:
	./liamc/bin/liamc/liamc --codegen --include=liamc/runtime --stdlib=liamc/stdlib --out="liamc/bin/liamc/stage2.exe" --in="liamc/selfhost/main.liam" --time --emit="liamc/bin/liamc/out.cpp" --debug
	@clang-format -i liamc/bin/liamc/out.cpp
	./liamc/bin/liamc/stage2.exe

selfhost-test:
	./liamc/bin/liamc/liamc --test --codegen --include=liamc/runtime --stdlib=liamc/stdlib --out="liamc/bin/liamc/stage2.exe" --in="liamc/selfhost/main.liam" --time --emit="liamc/bin/liamc/out.cpp" --debug
	@clang-format -i liamc/bin/liamc/out.cpp
	./liamc/bin/liamc/stage2.exe
