# компилятор
CC = gcc

# флаги
CFLAGS = -Wall -Wextra -std=c11

# библиотеки
LIBS = -lrt

# исполняемые файлы
TARGETS = client1 client2

# правило по умолчанию
all: $(TARGETS)

# сборка client1
client1: client1.c
	$(CC) $(CFLAGS) client1.c -o client1 $(LIBS)

# сборка client2
client2: client2.c
	$(CC) $(CFLAGS) client2.c -o client2 $(LIBS)

# очистка
clean:
	rm -f $(TARGETS)

# пересборка
rebuild: clean all