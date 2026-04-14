#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>

#define Q1 "/q1"
#define Q2 "/q2"
#define SIZE 256
#define EXIT_PRIO 10

int main() {
    mqd_t q1, q2;// очередь для получения и отправки
    char buf[SIZE];
    unsigned prio; // сюда запишется приоритет полученного сообщения

    q1 = mq_open(Q1, O_RDWR); //флаг читать и писать
    q2 = mq_open(Q2, O_RDWR);

    while (1) {
        // прием
        mq_receive(q1, buf, SIZE, &prio); // ждем пока кто то отправит сообщение 
        printf("Other: %s", buf); // выводим сообщение собеседника, как он лежит в буфе?

        if (prio == EXIT_PRIO) break; //если приоритет 10 выходим 

        // ответ
        printf("You: ");
        fgets(buf, SIZE, stdin);

        int p = 1; //установка приоритета
        if (strncmp(buf, "exit", 4) == 0) //сравнивает перевые 4 символа
            p = EXIT_PRIO;

        mq_send(q2, buf, strlen(buf)+1, p);

        if (p == EXIT_PRIO) break;
    }

    mq_close(q1);
    mq_close(q2);

    return 0;
}