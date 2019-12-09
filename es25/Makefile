CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es25

es25: es25.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es25
