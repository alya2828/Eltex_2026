#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

//./client  57123 или 127.0.0.1 

/* Старый клиент зависел от сервера, recv
Новый клиент не ждет,  сам иницирует*/

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int my_sock, portno, n;
    struct sockaddr_in serv_addr; // адрес сервера
    struct hostent *server;       //??

    // char buff[256]; для сообщений
    char buff[1024];
    printf("TCP DEMO CLIENT\n");

    if (argc < 3)
    {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
    // извлечение порта
    portno = atoi(argv[2]);

    // Шаг 1 - создание сокета
    my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock < 0)
        error("ERROR opening socket");
    // извлечение хоста
    server = gethostbyname(argv[1]); // переводим в IP
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    // заполенние структуры serv_addr
    memset(&serv_addr, 0, sizeof(serv_addr)); // Очищам структуру
    serv_addr.sin_family = AF_INET;           // IP_V4
    memcpy(&serv_addr.sin_addr.s_addr,        // Копируем IP адрес
           server->h_addr_list[0],
           server->h_length);
    // установка порта
    serv_addr.sin_port = htons(portno);

    // Шаг 2 - подключаемся к серверу
    if (connect(my_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    // Шаг 3 - чтение и передача сообщений
    while (1)
    {
        printf("Enter: a b op: ");

        fgets(buff, sizeof(buff), stdin);

        send(my_sock, buff, strlen(buff), 0); // отправляем данные

        int n = recv(my_sock, buff, sizeof(buff) - 1, 0); // ответ сервера

        if (n <= 0)
            break;

        buff[n] = '\0';
        printf("Server: %s", buff); // показываем ответ
    }
    printf("Recv error \n");
    close(my_sock);
    return -1;
}