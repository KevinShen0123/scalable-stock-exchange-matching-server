CC=g++
CFLAGS=-g -O0
EXTRAFLAGS=-lpqxx -lpq

all: testT

testT:querys.cpp
	$(CC) $(CFLAGS) -o testT querys.cpp $(EXTRAFLAGS)

clean:
	rm -f *~ *.o testT

clobber:
	rm -f *~ *.o