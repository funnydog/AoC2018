CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es18

es18: es18.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es18
