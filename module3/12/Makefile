CC = gcc
CFLAGS = -Wall -Wextra -std=c11

TARGET = chat

all: $(TARGET)

$(TARGET): chat.c
	$(CC) $(CFLAGS) -o $(TARGET) chat.c -lrt

clean:
	rm -f $(TARGET)