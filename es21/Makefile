CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es21

es21: es21.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es21
