/* Пример простого TCP сервера
   Порт является аргументом, для запуска сервера неободимо ввести:
   # ./[имя_скомпилированного_файла] [номер порта]
   # ./server 57123
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// прототип функции, обслуживающей
// подключившихся пользователей
void dostuff(int);

// Функция обработки ошибок
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

// глобальная переменная – количество
// активных пользователей
int nclients = 0;

// макрос для печати количества активных
// пользователей
void printusers()
{
    if (nclients)
    {
        printf("%d user on-line\n", nclients);
    }
    else
    {
        printf("No User on line\n");
    }
}

// функция по варианту +
int myfunc(char x, int a, int b) // нужен ли break
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
    // char buff[1024];    // Буфер для различных нужд
    printf("TCP SERVER DEMO\n");

    int sockfd, newsockfd;                  // дескрипторы сокетов
    int portno;                             // номер порта
    int pid;                                // id номер потока
    socklen_t clilen;                       // размер адреса клиента типа socklen_t
    struct sockaddr_in serv_addr, cli_addr; // структура сокета сервера и клиента

    // ошибка в случае если мы не указали порт
    if (argc < 2)
    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }
    // Шаг 1 - создание сокета
    // AF_INET     - сокет Интернета
    // SOCK_STREAM  - потоковый сокет (с
    //      установкой соединения)
    // 0 - по умолчанию выбирается TCP протокол
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // ошибка при создании сокета
    if (sockfd < 0)
        error("ERROR opening socket");

    // Шаг 2 - связывание сокета с локальным адресом
    memset(&serv_addr, 0, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; // сервер принимает подключения на все IP-адреса
    serv_addr.sin_port = htons(portno);
    // вызываем bind для связывания
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    // Шаг 3 - ожидание подключений, размер очереди - 5
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    // Шаг 4 - извлекаем сообщение из очереди
    // цикл извлечения запросов на подключение из очереди
    while (1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");
        nclients++; // увеличиваем счетчик
                    // подключившихся клиентов
                    /*
                    // вывод сведений о клиенте
                    struct hostent *hst;
                    hst = gethostbyaddr((char *)&cli_addr.sin_addr, 4, AF_INET);
                    printf("+%s [%s] new connect!\n",
                    (hst) ? hst->h_name : "Unknown host",
                    (char*)inet_ntoa(cli_addr.sin_addr));*/
        printusers();

        pid = fork();
        if (pid < 0)
            error("ERROR on fork");
        if (pid == 0)
        {
            close(sockfd);
            dostuff(newsockfd);
            exit(0);
        }
        else
            close(newsockfd);
    } /* end of while */
    close(sockfd);
    return 0; /* we never get here */
}

void dostuff(int sock)
{
    int bytes_recv;
    int a, b;
    char x;
    char buff[1024];

    // 1 число
    write(sock, "Enter 1 parametr\n", 18);

    bytes_recv = read(sock, buff, sizeof(buff) - 1);
    if (bytes_recv <= 0) return;
    buff[bytes_recv] = '\0';
    a = atoi(buff);

    // 2 число
    write(sock, "Enter 2 parametr\n", 18);

    bytes_recv = read(sock, buff, sizeof(buff) - 1);
    if (bytes_recv <= 0) return;
    buff[bytes_recv] = '\0';
    b = atoi(buff);
int valid = 0;


while (valid == 0)
{
    write(sock, "Enter operation (+ - * /):\n",
          strlen("Enter operation (+ - * /):\n"));

    bytes_recv = read(sock, buff, sizeof(buff) - 1);
    if (bytes_recv <= 0) return;

    buff[bytes_recv] = '\0';

    sscanf(buff, " %c", &x);

    if (x == '+' || x == '-' || x == '*' || x == '/')
    {
        valid = 1;
    }
    else
    {
        write(sock,
              "ERROR: invalid operation. Try again.\n",
              strlen("ERROR: invalid operation. Try again.\n"));
    }
}
    // результат
    int result = myfunc(x, a, b);

    snprintf(buff, sizeof(buff), "Result: %d\n", result);
    write(sock, buff, strlen(buff));

    // файл
    write(sock, "Send file or type END:\n",
          strlen("Send file or type END:\n"));

    FILE *fp = fopen("received.txt", "wb");
    if (!fp)
    {
        perror("file error");
        return;
    }

    while (1)
    {
        bytes_recv = read(sock, buff, sizeof(buff));

        if (bytes_recv <= 0)
            break;

        if (strstr(buff, "END"))
            break;

        fwrite(buff, 1, bytes_recv, fp);
    }

    fclose(fp);

    printf("File received\n");

    nclients--;
    printf("-disconnect\n");
    printusers();
    return;

}