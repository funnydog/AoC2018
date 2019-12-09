CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es12

es12: es12.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es12 es12b
