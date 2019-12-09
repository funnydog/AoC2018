CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es2 es2b

es2: es2.o
	$(CC) $(CFLAGS) -o $@ $^

es2b: es2b.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es2 es2b
