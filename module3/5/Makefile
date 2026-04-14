CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2

TARGET = signals
SRC = signals.c
OUTPUT = output.txt

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OUTPUT)