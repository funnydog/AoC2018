CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es22

es22: es22.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es22
