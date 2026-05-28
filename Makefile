CC = gcc
CFLAGS = -Wall -std=c89

all: battleship

battleship: main.o reset.o
	$(CC) $(CFLAGS) -o battleship main.o reset.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

reset.o: reset.c
	$(CC) $(CFLAGS) -c reset.c

clean:
	rm -f battleship main.o reset.o
