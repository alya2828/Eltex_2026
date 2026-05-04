/* Пример простого TCP сервера
   Порт является аргументом, для запуска сервера неободимо ввести:
   # ./server 57123
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>

// Функция обработки ошибок
void error(const char *msg)
{
    perror(msg); // печатает msg + системную ошибку
    exit(1);     // завершает программу
}

//  Функция калькулятора
int myfunc(char x, int a, int b)
{
    switch (x)
    {
    case '+':
        return a + b;
    case '-':
        return a - b;
    case '*':
        return a * b;
    case '/':
        if (b == 0)
            return 0;
        return a / b;
    default:
        printf("Error: invalid operation '%c'\n", x);
        return 1;
    }
}

int main(int argc, char *argv[])
{
    int listener, newfd, port;
    // listener — серверный сокет
    // newfd — сокет клиента
    // port — порт

    struct sockaddr_in serv_addr, cli_addr;
    // serv_addr — адрес сервера
    // cli_addr — адрес клиента

    socklen_t clilen;
    // размер структуры адреса клиента

    fd_set master, read_fds;
    // master — ВСЕ сокеты (сервер + клиенты)
    // read_fds — временный набор для select

    int fdmax;
    // максимальный файловый дескриптор

    char buffer[1024];
    // буфер для данных

    // Проверка аргументов
    if (argc < 2)
        error("No port");

    // Создание сокета
    listener = socket(AF_INET, SOCK_STREAM, 0);
    // AF_INET — IPv4
    // SOCK_STREAM — TCP

    if (listener < 0)
        error("socket");

    //  Обнуляем структуру адреса
    memset(&serv_addr, 0, sizeof(serv_addr));

    // Получаем порт из аргументов
    port = atoi(argv[1]);

    //  Настраиваем адрес сервера
    serv_addr.sin_family = AF_INET;         // IPv4
    serv_addr.sin_addr.s_addr = INADDR_ANY; // любой IP
    serv_addr.sin_port = htons(port);       // порт (в сетевом порядке)

    // Привязываем сокет к адресу
    if (bind(listener, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("bind");

    // Начинаем слушать подключения
    listen(listener, 5);
    // 5 — очередь подключений

    // Инициализация select

    FD_ZERO(&master);
    // очищаем множество

    FD_SET(listener, &master);
    // добавляем серверный сокет

    fdmax = listener;
    // пока максимум = сервер

    printf("Server started...\n");

    while (1)
    {
        read_fds = master; // копируем список всех сокетов(сервер,клиент1,клиент2...)
        // потому что select изменяет набор

        // Ожидание событий
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) < 0)
            error("select");

        // Проверяем все дескрипторы
        for (int i = 0; i <= fdmax; i++)
        {
            // Проверяем: есть ли событие на этом fd
            if (FD_ISSET(i, &read_fds))
            /*FD_ISSET(int fd, fd_set *set): проверка, находится ли файловый
    дескриптор fd в fd_set. Если это так, функция возвращает ненулевое
    значение, в противном случае она возвращает 0.*/
            {
                // если это сервер, пришел новый клиент
                // listener не меняется, создается только новый fd
                if (i == listener)
                {
                    clilen = sizeof(cli_addr); // выделяем место для нового клиента

                    // Принимаем нового клиента
                    newfd = accept(listener, (struct sockaddr *)&cli_addr, &clilen);
                    // Принимаем подключение, создаем новый сокет, возращаем его дискриптор
                    if (newfd < 0)
                        error("accept");

                    //  Добавляем клиента в список master
                    FD_SET(newfd, &master);

                    //  Обновляем максимум
                    if (newfd > fdmax)
                        fdmax = newfd;

                    printf("New client connected: %d\n", newfd);
                }
                else // это не listener, а клиент прислал данные
                {
                    // Данные от клиента

                    int nbytes = recv(i, buffer, sizeof(buffer) - 1, 0);
                    // читаем данные

                    if (nbytes <= 0)
                    {
                        //  клиент отключился
                        close(i);           // закрываем сокет
                        FD_CLR(i, &master); // удаляем из списка
                        printf("Client %d disconnected\n", i);
                    }
                    else
                    {
                        // данные получены
                        buffer[nbytes] = '\0';
                        // делаем строку из массива байт

                        printf("Client %d: %s", i, buffer);

                        // парсинг
                        int a, b;
                        char op;

                        if (sscanf(buffer, "%d %d %c", &a, &b, &op) == 3)
                        {
                            // если корректный формат

                            int res = myfunc(op, a, b);

                            char reply[100];

                            sprintf(reply, "Result: %d\n", res);
                            // записывает ответ в строку

                            send(i, reply, strlen(reply), 0);
                            // отправляем клиенту
                        }
                        else
                        {
                            // неправильный формат
                            char *msg = "Format: a b op\n";
                            send(i, msg, strlen(msg), 0);
                        }
                    }
                }
            }
        }
    }

    return 0;
}
