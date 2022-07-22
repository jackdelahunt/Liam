format:
	@find . -type f \( -name "*.h" -or -name "*.cpp" \) | xargs clang-format -i --style=file:".clang-format"

run:
	./liamc/bin/liamc/liamc --include=liamc/runtime --out="liamc/Code/out.exe" --in="liamc/Code/main.liam" --time
	./liamc/Code/out.exe