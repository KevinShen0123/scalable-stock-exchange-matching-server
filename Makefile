CC=g++
CFLAGS=-g -O0
EXTRAFLAGS=-lpqxx -lpq

all: test

test:querys.cpp
	$(CC) $(CFLAGS) -o test querys.cpp $(EXTRAFLAGS)

clean:
	rm -f *~ *.o test

clobber:
	rm -f *~ *.o