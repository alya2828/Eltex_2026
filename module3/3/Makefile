CC = gcc
CFLAGS = -Wall -Wextra -std=c11

all: contacts_app

contacts_app: menu.o contacts.o
	$(CC) $(CFLAGS) menu.o contacts.o -o contacts_app

menu.o: menu.c contacts.h
	$(CC) $(CFLAGS) -c menu.c

contacts.o: contacts.c contacts.h
	$(CC) $(CFLAGS) -c contacts.c

clean:
	rm -f *.o contacts_app contacts.dat