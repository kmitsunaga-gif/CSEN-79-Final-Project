# Makefile - one-command build for the CSEN 79 order book project.
#
#   make          build the app, the unit tests, and the workload generators
#   make test     build, then run every unit test
#   make sample   build, then check the app against samples/p1_demo (no output from diff = pass)
#   make asan     build with AddressSanitizer + UBSan, then run the tests and the sample under it
#   make fast     build with counters compiled out (for pure timings)
#   make clean    delete every build folder

.PHONY: all test sample asan fast clean

all:
	cmake -S . -B build
	cmake --build build -j

test: all
	./build/tests

sample: all
	./build/app < samples/p1_demo.txt | grep -v "^STATS" | diff - samples/p1_demo.expected.txt
	@echo "sample: PASS (output matches samples/p1_demo.expected.txt)"

asan:
	cmake -S . -B build-asan -DSANITIZE=ON
	cmake --build build-asan -j
	./build-asan/tests
	if [ -x build-asan/app ]; then ./build-asan/app < samples/p1_demo.txt > /dev/null; fi
	@echo "asan: no errors reported"

fast:
	cmake -S . -B build-fast -DCOUNTERS=OFF
	cmake --build build-fast -j

clean:
	rm -rf build build-asan build-fast