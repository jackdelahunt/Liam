format:
	@find . -type f \( -name "*.h" -or -name "*.cpp" \) | xargs clang-format -i --style=Microsoft