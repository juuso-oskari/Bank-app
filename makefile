#the compiler
CC=gcc
#compiler flags
CFLAGS=-Wall -pedantic -lpthread

DEPS = account.h bank.h miscellanous.h queue.h server.h
OBJ_1 = TCPserver.o account.o bank.o miscellanous.o queue.o server.o
OBJ_2 = TCPclient.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: server_executable executable clean

executable: $(OBJ_2)
	$(CC) -o $@ $^ $(CFLAGS) -L -lpthread

server_executable: $(OBJ_1)
	$(CC) -o $@ $^ $(CFLAGS) -L -lpthread

.PHONY: clean

clean:
	rm -f *.o *~ core $(INCDIR)/*~
