CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es13 es13b

es13: es13.o
	$(CC) $(CFLAGS) -o $@ $^

es13b: es13b.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es13 es13b
