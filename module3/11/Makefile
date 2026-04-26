CC = gcc
CFLAGS = -Wall -Wextra -std=c11

TARGET = prog

all: $(TARGET)

$(TARGET): prog.c
	$(CC) $(CFLAGS) -o $(TARGET) prog.c -lrt

clean:
	rm -f $(TARGET)