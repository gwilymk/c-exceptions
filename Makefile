CFLAGS = -std=gnu99 -Wall -Wextra -g
H_FILES = $(shell find -name '*.h')
C_FILES = $(shell find -name '*.c')

.PHONY: default
default: test

exceptions_test: Makefile $(C_FILES) $(H_FILES)
	$(CC) $(CFLAGS) -o exceptions_test $(C_FILES)

.PHONY: test
test: exceptions_test
	./exceptions_test