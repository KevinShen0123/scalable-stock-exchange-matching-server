CC=g++
CFLAGS=-g -O0
EXTRAFLAGS=-lpqxx -lpq -lpthread

all: testServer

testServer:XML.cpp  pugiXML/pugixml.cpp querys.cpp Server.cpp
	$(CC) $(CFLAGS) -o testServer XML.cpp  pugiXML/pugixml.cpp querys.cpp Server.cpp $(EXTRAFLAGS)
clean:
	rm -f *~ *.o testServer

clobber:
	rm -f *~ *.o