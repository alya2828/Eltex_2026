CC = gcc

CFLAGS = -Wall -Wextra -pthread

SERVER = server
CLIENT = main

SERVER_SRC = server.c
CLIENT_SRC = main.c

all: $(SERVER) $(CLIENT)

$(SERVER):
	$(CC) $(CFLAGS) $(SERVER_SRC) -o $(SERVER)

$(CLIENT):
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o $(CLIENT)

clean:
	rm -f $(SERVER) $(CLIENT)