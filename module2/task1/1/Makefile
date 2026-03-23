
CC = gcc

all: menu build

contacts: contacts.o
	$(CC) contacts.o -o ./contacts.out

menu: contacts.o menu.o 
	$(CC) contacts.o menu.o -o ./menu.out

contacts.o: 
	$(CC) -c contacts.c

menu.o:
	$(CC) -c menu.c

build: 
	./menu.out

clean: 
	rm -rf *.o