.PHONY: test clean

CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror -pedantic -O2

test: build/test_matrix
	./build/test_matrix

build/test_matrix: common/matrix.c common/matrix.h tests/test_matrix.c
	mkdir -p build
	$(CC) $(CFLAGS) -Icommon common/matrix.c tests/test_matrix.c -o $@

clean:
	rm -rf build

