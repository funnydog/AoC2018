CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es1 es1b

es1: es1.o
	$(CC) $(CFLAGS) -o $@ $^

es1b: es1b.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es1 es1b
