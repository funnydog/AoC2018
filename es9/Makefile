CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es9

es9: es9.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es9 es9b
