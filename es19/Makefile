CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es19

es19: es19.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es19
