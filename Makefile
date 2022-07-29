format:
	@find . -type f \( -name "*.h" -or -name "*.cpp" \) | xargs clang-format -i --style=file:".clang-format"

run:
	./liamc/bin/liamc/liamc --emit="liamc/Code/out.cpp" --include=liamc/runtime --stdlib=liamc/stdlib --out="liamc/Code/out.exe" --in="liamc/Code/main.liam" --time --debug
	./liamc/Code/out.exe
