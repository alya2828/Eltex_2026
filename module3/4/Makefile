# Компилятор
CC = gcc

# Флаги компиляции
CFLAGS = -Wall -Wextra -std=c11

# Имя программы (твой shell)
TARGET = process2

all: $(TARGET)

# Сборка
$(TARGET): process2.c
	$(CC) $(CFLAGS) -o $(TARGET) process2.c

# Запуск (удобно для тестов)
run: $(TARGET)
	./$(TARGET)

# Очистка
clean:
	rm -f $(TARGET)