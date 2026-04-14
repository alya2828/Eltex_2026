# Makefile для твоего командного интерпретатора и тестовых программ

CC = gcc
CFLAGS = -Wall -Wextra -std=c11

# Имя исполняемых файлов
TARGETS = process1 sum text max

all: $(TARGETS)

# Компиляция process1
process1: process1.c
	$(CC) $(CFLAGS) -o process1 process1.c

# Компиляция sum для примера
sum: sum.c
	$(CC) $(CFLAGS) -o sum sum.c

# Компиляция text для примера
text: text.c
	$(CC) $(CFLAGS) -o text text.c

# Компиляция max для примера
max: max.c
	$(CC) $(CFLAGS) -o max max.c

# Очистка файлов для примера
clean:
	rm -f $(TARGETS)
