CC = clang
CFLAGS = -Wall -march=native -O3 -DSSE2NEON_SUPPRESS_WARNINGS -std=c99

.PHONY: clean test bench

HEADERS = $(wildcard *.h)
TESTS = $(wildcard tests/*.c)
BENCHMARKS = $(wildcard bench/*.c)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TESTS)
	$(CC) $(CFLAGS) $(TESTS) -o test
	./test

bench: $(TESTS)
	$(CC) $(CFLAGS) $(BENCHMARKS) -o benchmark
	./benchmark

clean:
	-rm -f *.o
	-rm -f test
	-rm -f benchmark
