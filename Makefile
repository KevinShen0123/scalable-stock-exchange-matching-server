CFLAGS=-std=gnu++11 -pedantic -ggdb3
EXTRAFLAGS=-lpqxx -lpq #-pthread

all: test

test: main.cpp pugixml/pugixml.cpp
	g++ $(CFLAGS) -o test main.cpp pugixml/pugixml.cpp

clean:
	rm -f *~ *.o test


