CFLAGS=-Wall -g -ggdb

.PHONY: clean all

all: es20

es20: es20.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o es20
