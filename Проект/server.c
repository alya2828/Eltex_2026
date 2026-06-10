#include <stdio.h>             
#include <stdlib.h>     
#include <unistd.h>            
#include <string.h>            
#include <pthread.h>            
#include <arpa/inet.h>          
#include <sys/socket.h>         

#include "server.h" // общий заголовок сервера (команды, порты, лимиты)

// массив сокетов всех подключённых клиентов
int clients[MAX_CLIENTS];

// текущее количество подключённых клиентов
int client_count = 0;

// мьютекс для защиты массива клиентов (чтобы потоки не ломали данные)
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// ПОТОК ПРИЁМА КЛИЕНТОВ

void *accept_thread(void *arg)
{

    int server_socket = *(int *)arg;

    struct sockaddr_in client_addr;


    socklen_t len = sizeof(client_addr);


    while (1)
    {
        // принимаем новое подключение 
        int client_socket = accept(server_socket,(struct sockaddr *)&client_addr, &len );

        if (client_socket < 0)
            continue;

        // блокируем доступ к общим данным (clients[])
        pthread_mutex_lock(&clients_mutex);

        // проверяем, есть ли место под нового клиента
        if (client_count < MAX_CLIENTS)
        {
            // сохраняем сокет клиента в массив
            clients[client_count] = client_socket;

            // увеличиваем счётчик клиентов
            client_count++;

            // вывод информации
            printf("[SERVER] Client connected: %d\n", client_count);
        }
        else
        {
            // если клиентов слишком много — отклоняем
            printf("[SERVER] Max clients reached\n");

            // закрываем лишний сокет
            close(client_socket);
        }

        // разблокируем доступ к массиву клиентов
        pthread_mutex_unlock(&clients_mutex);
    }

    // завершение потока (никогда не достигнется))))
    return NULL;
}



// Отправляем всем клиентам команды
void send_all(command_t cmd)
{
    // блокируем доступ к массиву клиентов
    pthread_mutex_lock(&clients_mutex);

    // проходим по всем подключённым клиентам
    for (int i = 0; i < client_count; i++)
    {
        // отправляем команду по TCP каждому клиенту
        send(clients[i], &cmd, sizeof(cmd), 0);
    }

    // разблокируем доступ
    pthread_mutex_unlock(&clients_mutex);
}


int main()
{
    int server_socket;              // главный серверный сокет
    struct sockaddr_in server_addr; // структура адреса сервера

    // создаём TCP сокет 
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0)
    {
        printf("Socket error\n");
        return 1;
    }

    server_addr.sin_family = AF_INET;

    // принимаем подключения с любого IP
    server_addr.sin_addr.s_addr = INADDR_ANY;

    server_addr.sin_port = htons(PORT);

    // привязываем сокет к IP и порту
    if (bind(server_socket, (struct sockaddr *)&server_addr,sizeof(server_addr)) < 0)
    {
        printf("Bind error\n");
        return 1;
    }

    // переводим сокет в режим ожидания клиентов
    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        printf("Listen error\n");
        return 1;
    }

    // сообщение о запуске сервера
    printf("[SERVER] Started on port %d\n", PORT);

    // создаём поток, который будет принимать клиентов
    pthread_t tid;
    pthread_create(&tid, NULL, accept_thread, &server_socket);

    // Меню сервера 
    while (1)
    {
        int cmd; 
        printf("\n===== MENU =====\n");
        printf("1 - START LOAD\n");
        printf("2 - STOP LOAD\n");
        printf("3 - SEND FILE\n");
        printf("4 - STATUS\n");
        printf("5 - EXIT\n");
        printf("> ");

        // ввод команды
        scanf("%d", &cmd);

        // обработка команды
        switch (cmd)
        {
        case 1:
            // запуск нагрузки
            printf("[SERVER] START\n");

            // отправка команды всем клиентам
            send_all(CMD_START);
            break;

        case 2:
            // остановка нагрузки
            printf("[SERVER] STOP\n");

            // отправка STOP всем клиентам
            send_all(CMD_STOP);
            break;

        case 3:
            // отправка файла (пока только команда)
            printf("[SERVER] SEND FILE\n");

            // уведомляем клиентов о файле
            send_all(CMD_SEND_FILE);
            break;

        case 4:
            // запрос статуса
            printf("[SERVER] STATUS\n");

            // отправка запроса статуса
            send_all(CMD_STATUS);
            break;

        case 5:
            // завершение работы сервера
            printf("[SERVER] EXIT\n");

            // отправка команды выхода всем клиентам
            send_all(CMD_EXIT);

            // завершение программы
            exit(0);

        default:
            // неизвестная команда
            printf("Unknown command\n");
            break;
        }
    }

    return 0;
}