CC=g++
CFLAGS=-g -O0
EXTRAFLAGS=-lpqxx -lpq

all: Server

testT:querys.cpp
	$(CC) $(CFLAGS) -o Server Server.cpp $(EXTRAFLAGS)

clean:
	rm -f *~ *.o testT

clobber:
	rm -f *~ *.o