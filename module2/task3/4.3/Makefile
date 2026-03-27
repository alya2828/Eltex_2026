
CC = gcc

all: menu build

tree_contacts: tree_contacts.o
	$(CC) tree_contacts.o -o ./tree_contacts.out

menu: tree_contacts.o menu.o 
	$(CC) tree_contacts.o menu.o -o ./menu.out

tree_contacts.o: 
	$(CC) -c tree_contacts.c

menu.o:
	$(CC) -c menu.c

build: 
	./menu.out

clean: 
	rm -rf *.o