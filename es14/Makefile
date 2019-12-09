CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es14

es14: es14.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es14
