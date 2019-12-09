CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es15

es15: es15.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es15
