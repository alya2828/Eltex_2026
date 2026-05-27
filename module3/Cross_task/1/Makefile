CC = gcc
CFLAGS = -Wall -Wextra -O2

TARGETS = client server

all: $(TARGETS)

client: client.c
	$(CC) $(CFLAGS) client.c -o client

server: server.c
	$(CC) $(CFLAGS) server.c -o server

clean:
	rm -f $(TARGETS)