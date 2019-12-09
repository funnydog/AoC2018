CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es10

es10: es10.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es10 es10b
