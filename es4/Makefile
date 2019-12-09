CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es4

es4: es4.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es4
