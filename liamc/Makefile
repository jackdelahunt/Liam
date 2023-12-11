format:
	@find . -type f \( -name "*.h" -or -name "*.cpp" \) | xargs clang-format -i --style=file:".clang-format"

release:
	mkdir release
	mv build/release/liamc release
	cp -r stdlib release
	cp -r core release
