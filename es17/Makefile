CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es17 es17b

es17: es17.o
	$(CC) $(CFLAGS) -o $@ $^

es17b: es17b.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es17 es17b
