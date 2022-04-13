.PHONY: build
build:
	clear
	rm -rf build/
	mkdir build
	(cd build/ && cmake -DCMAKE_CXX_COMPILER="/usr/bin/clang++" ../)
	(cd build/ && make)

.PHONY: build_debug
build_debug:
	clear
	rm -rf build/
	mkdir build
	(cd build/ && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER="/usr/bin/clang++" ../)
	(cd build/ && make)


.PHONY: run
run:
	(cd build/ && ./liamvm)