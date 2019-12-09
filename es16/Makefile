CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es16

es16: es16.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es16
