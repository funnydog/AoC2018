CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es6

es6: es6.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es6 es6b
