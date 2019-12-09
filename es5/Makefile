CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es5

es5: es5.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es5 es5b
