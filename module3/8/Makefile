CC = gcc
CFLAGS = -Wall -Wextra -std=c11

TARGETS = producer consumer

all: $(TARGETS)

producer: producer.c
	$(CC) $(CFLAGS) -o producer producer.c

consumer: consumer.c
	$(CC) $(CFLAGS) -o consumer consumer.c

clean:
	rm -f $(TARGETS) *.o