CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es7

es7: es7.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es7 es7b
