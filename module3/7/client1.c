#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>

#define Q1 "/q1"
#define Q2 "/q2"
#define SIZE 256
#define EXIT_PRIO 10

int main()
{
    mqd_t q1, q2;
    char buf[SIZE];
    unsigned prio;

    struct mq_attr attr = {0};
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = SIZE;

    q1 = mq_open(Q1, O_CREAT | O_RDWR, 0666, &attr);
    q2 = mq_open(Q2, O_CREAT | O_RDWR, 0666, &attr);

    while (1)
    {
        // отправка
        printf("You: ");
        fgets(buf, SIZE, stdin);

        int p = 1;
        if (strncmp(buf, "exit", 4) == 0)
            p = EXIT_PRIO;

        mq_send(q1, buf, strlen(buf) + 1, p);

        if (p == EXIT_PRIO)
            break;

        // прием
        mq_receive(q2, buf, SIZE, &prio);
        printf("Other: %s", buf);

        if (prio == EXIT_PRIO)
            break;
    }

    mq_close(q1);
    mq_close(q2);
    mq_unlink(Q1);
    mq_unlink(Q2);

    return 0;
}
// проверить что очередь в ядре создалась - ls /dev/mqueue