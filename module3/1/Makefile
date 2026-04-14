# Имя исполняемого файла
TARGET = process

# Исходный код
SRC = process.c

# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -Wextra -std=c11

# Правило по умолчанию
all: $(TARGET)

# Как собирать исполняемый файл
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Удаление скомпилированного файла
clean:
	rm -f $(TARGET)