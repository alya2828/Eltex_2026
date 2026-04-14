#include "message.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include "message.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <client_id>\n", argv[0]);
        return 1;
    }

    int client_id = atoi(argv[1]);

    key_t key = ftok("chat", 65);
    if (key == -1)
    {
        perror("ftok");
        exit(1);
    }

    int msqid = msgget(key, 0666);
    if (msqid == -1)
    {
        perror("msgget");
        exit(1);
    }

    struct message msg;

    printf("Client %d started\n", client_id);

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        exit(1);
    }

    if (pid == 0)
    {
        // получение
        while (1)
        {
            if (msgrcv(msqid, &msg, sizeof(msg) - sizeof(long), client_id, 0) == -1)
            {
                perror("msgrcv");
                continue;
            }
            printf("\nFrom %d: %s\n", msg.sender, msg.text);
        }
    }
    else
    {
        // отправка
        while (1)
        {
            msg.mtype = SERVER_TYPE;
            msg.sender = client_id;

            printf("You: ");
            fflush(stdout);
            fgets(msg.text, MAX_TEXT, stdin);
            msg.text[strcspn(msg.text, "\n")] = 0;
            if (strlen(msg.text) == 0)
                continue;

            if (msgsnd(msqid, &msg, sizeof(msg) - sizeof(long), 0) == -1)
            {
                perror("msgsnd");
            }

            if (strcmp(msg.text, "shutdown") == 0)
            {
                break;
            }
        }
    }

    return 0;
}