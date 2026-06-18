#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 5000
#define MAX_CLIENTS 10

typedef enum
{
    CMD_START_CPU = 1,
    CMD_START_RAM,
    CMD_START_ALL,
    CMD_STOP,
    CMD_SEND_FILE,
    CMD_STATUS,
    CMD_EXIT
} command_t;

typedef struct
{
    struct sockaddr_in addr;
    time_t connect_time;
} client_info_t;


// глобальные переменные 
extern int clients[MAX_CLIENTS];
extern int client_count;
extern pthread_mutex_t clients_mutex;

extern client_info_t clients_info[MAX_CLIENTS];


void *accept_thread(void *arg);
void send_all(command_t cmd);

#endif