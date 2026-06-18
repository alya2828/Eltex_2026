#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h> 

#include "server.h"

int clients[MAX_CLIENTS];

client_info_t clients_info[MAX_CLIENTS];

int client_count = 0;

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void *accept_thread(void *arg)
{
    int server_socket = *(int *)arg;

    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

    while (1)
    {
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &len);

        if (client_socket < 0)
        {
            perror("[SERVER] accept");
            continue;
        }

        pthread_mutex_lock(&clients_mutex);

        if (client_count < MAX_CLIENTS)
        {
            int client_id = client_count + 1;

            clients[client_count] = client_socket;
            client_count++;

            send(client_socket, &client_id, sizeof(client_id), 0);

            printf("[SERVER] Client connected: client_%d IP=%s\n", client_id, inet_ntoa(client_addr.sin_addr));
        }
        else
        {
            printf("[SERVER] Max clients reached\n");
            close(client_socket);
        }

        pthread_mutex_unlock(&clients_mutex);
    }

    return NULL;
}

void send_all(command_t cmd)
{
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < client_count; i++)
    {
        ssize_t ret = send(clients[i], &cmd, sizeof(cmd), 0);

        if (ret <= 0)
        {
            printf("[SERVER] Failed to send command to client %d\n", i + 1);
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

int main(void)
{
    int server_socket;
    struct sockaddr_in server_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0)
    {
        perror("[SERVER] socket");
        return 1;
    }

    int opt = 1;

    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("[SERVER] bind");
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        perror("[SERVER] listen");
        close(server_socket);
        return 1;
    }

    printf("[SERVER] Started on port %d\n", PORT);

    pthread_t tid;

    if (pthread_create(&tid, NULL, accept_thread, &server_socket) != 0)
    {
        perror("[SERVER] pthread_create");
        close(server_socket);
        return 1;
    }

    while (1)
    {
        int cmd;

        printf("\n===== MENU =====\n");
        printf("1 - START CPU LOAD\n");
        printf("2 - START RAM LOAD\n");
        printf("3 - START ALL LOAD\n");
        printf("4 - STOP LOAD\n");
        printf("5 - SEND FILE\n");
        printf("6 - STATUS\n");
        printf("7 - EXIT\n");
        printf("8 - SHOW CLIENTS\n");
        printf("> ");

        if (scanf("%d", &cmd) != 1)
        {
            printf("[SERVER] Input error\n");
            break;
        }

        switch (cmd)
        {
        case 1:
            printf("[SERVER] START CPU LOAD\n");
            send_all(CMD_START_CPU);
            break;
        case 2:
            printf("[SERVER] START RAM LOAD\n");
            send_all(CMD_START_RAM);
            break;
        case 3:
            printf("[SERVER] START ALL LOAD\n");
            send_all(CMD_START_ALL);
            break;
        case 4:
            printf("[SERVER] STOP\n");
            send_all(CMD_STOP);
            break;

        case 5:
            printf("[SERVER] SEND FILE\n");
            send_all(CMD_SEND_FILE);
            break;
        
        case 6:
            printf("[SERVER] STATUS\n");
            send_all(CMD_STATUS);
            break;

        case 7:
            printf("[SERVER] EXIT\n");
            send_all(CMD_EXIT);
            close(server_socket);
            exit(0);
        case 8:
        {
            pthread_mutex_lock(&clients_mutex);
            printf("\n--- Connected Clients (%d) ---\n", client_count);
            if (client_count == 0)
            {
                printf("No clients connected.\n");
            }
            else
            {
                for (int i = 0; i < client_count; i++)
                {
                    char ip_str[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &clients_info[i].addr.sin_addr, ip_str, sizeof(ip_str));
                    int port = ntohs(clients_info[i].addr.sin_port);

                    char time_buf[64];
                    struct tm *tm_info = localtime(&clients_info[i].connect_time);
                    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", tm_info);

                    printf("[%d] %s:%d  connected at %s  socket=%d\n",
                           i, ip_str, port, time_buf, clients[i]);
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            break;
        }

        default:
            printf("[SERVER] Unknown command\n");
            break;
        }
    }

    close(server_socket);

    return 0;
}