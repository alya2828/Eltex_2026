#include "message.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "message.h"

int main()
{
    key_t key = ftok("chat", 65);
    if (key == -1)
    {
        perror("ftok");
        exit(1);
    }

    int msqid = msgget(key, 0666 | IPC_CREAT); // создает очередь
    if (msqid == -1)
    {
        perror("msgget");
        exit(1);
    }

    struct message msg;
    int clients[MAX_CLIENTS] = {0};

    printf("Server started...\n");

    while (1)
    {
        msgrcv(msqid, &msg, sizeof(msg) - sizeof(long), SERVER_TYPE, 0);

        if (strlen(msg.text) == 0)
            continue;

        int sender = msg.sender;

        // регистрация клиента
        int exists = 0;
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i] == sender)
            {
                exists = 1;
                break;
            }
        }

        if (!exists)
        {
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i] == 0)
                {
                    clients[i] = sender;
                    printf("Client %d connected\n", sender);
                    break;
                }
            }
        }

        // shutdown
        if (strcmp(msg.text, "shutdown") == 0)
        {
            printf("Client %d disconnected\n", sender);
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i] == sender)
                {
                    clients[i] = 0;
                }
            }
            continue;
        }

        printf("From %d: %s\n", sender, msg.text);

        // рассылка
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i] != 0 && clients[i] != sender)
            {
                struct message out;
                out.mtype = clients[i];
                out.sender = sender;
                strcpy(out.text, msg.text);

                if (msgsnd(msqid, &out, sizeof(out) - sizeof(long), 0) == -1)
                {
                    perror("msgsnd");
                }
            }
        }
    }

    return 0;
}
//проверить что очередь создалась ipcs -q
