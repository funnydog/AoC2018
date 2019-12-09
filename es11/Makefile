CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es11

es11: es11.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es11 es11b
