CC = gcc
CFLAGS = -Wall -Wextra -std=c11

TARGET = snifer

all: $(TARGET)

$(TARGET): snifer.c
	$(CC) $(CFLAGS) -o $(TARGET) snifer.c -lrt

clean:
	rm -f $(TARGET)