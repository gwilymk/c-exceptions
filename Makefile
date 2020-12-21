CFLAGS = -std=gnu99 -Wall -Wextra -g

OBJS = exceptions.o test_helper.o exceptions_test.o

.PHONY: default
default: test

exceptions_test: $(OBJS)
	$(CC) $(CFLAGS) -o exceptions_test $(OBJS)

.PHONY: test
test: exceptions_test
	./exceptions_test