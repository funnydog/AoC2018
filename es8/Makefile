CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es8

es8: es8.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es8 es8b
