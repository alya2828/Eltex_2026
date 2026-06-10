#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include <arpa/inet.h>
#define PORT 5000
#define MAX_CLIENTS 10

typedef enum
{
    CMD_START = 1,
    CMD_STOP,
    CMD_SEND_FILE,
    CMD_STATUS,
    CMD_EXIT
} command_t;

// глобальные переменные 
extern int clients[MAX_CLIENTS];
extern int client_count;
extern pthread_mutex_t clients_mutex;

void *accept_thread(void *arg);
void send_all(command_t cmd);

#endif