.PHONY: build
build:
	clear
	rm -rf build/
	mkdir build
	(cd build/ && cmake ../)
	(cd build/ && make)

.PHONY: build_debug
build_debug:
	clear
	rm -rf build/
	mkdir build
	(cd build/ && cmake -DCMAKE_BUILD_TYPE=Debug ../)
	(cd build/ && make)


.PHONY: run
run:
	(cd build/ && ./liamvm)