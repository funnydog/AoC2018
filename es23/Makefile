CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es23

es23: es23.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es23
