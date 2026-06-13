CC = gcc

CFLAGS = -Wall -Wextra -pthread

SERVER = calculator
CLIENT = main

SERVER_SRC = calculator.c
CLIENT_SRC = main.c process_control.c

all: $(SERVER) $(CLIENT)

$(SERVER): $(SERVER_SRC)
	$(CC) $(CFLAGS) $(SERVER_SRC) -o $(SERVER)

$(CLIENT): $(CLIENT_SRC)
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o $(CLIENT)

clean:
	rm -f $(SERVER) $(CLIENT)
