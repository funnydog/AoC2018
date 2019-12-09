CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es24

es24: es24.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es24
