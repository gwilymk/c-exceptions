CFLAGS = -std=gnu99 -Wall -Wextra -g
C_FILES = $(shell find -name '*.c')

.PHONY: default
default: test

exceptions_test: $(C_FILES)
	$(CC) $(CFLAGS) -o exceptions_test $(C_FILES)

.PHONY: test
test: exceptions_test
	./exceptions_test