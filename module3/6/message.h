#ifndef MESSAGE_H
#define MESSAGE_H

#define SERVER_TYPE 10
#define MAX_TEXT 100
#define MAX_CLIENTS 10

struct message {
    long mtype;        // кому отправляем
    int sender;        // кто отправил
    char text[MAX_TEXT];
};

#endif