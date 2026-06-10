#include <stdio.h>     
#include <stdlib.h>    
#include <unistd.h>    
#include <signal.h>    
#include <arpa/inet.h> 


#define SERVER_IP "127.0.0.1" 
#define SERVER_PORT 5000 

// Флаг работы программы
static volatile int running = 1;

// Дескриптор TCP-сокета
static int server_socket = -1;

// Обработчик 
void signal_handler(int sig)
{
    (void)sig;   // Не используем параметр
    running = 0; // Останавливаем главный цикл
}

// Инициализация системы
int system_init(void)
{
    printf("[CLIENT] System initialization...\n");

    // Регистрируем обработчик
    signal(SIGINT, signal_handler);

    // Здесь позже будет:
    // logger_init();
    // config_load();
    // process_init();
    // control_init();

    return 0;
}

#include <arpa/inet.h>          
// Подключение к серверу
int connect_to_server(void)
{
    
    struct sockaddr_in server_addr;

    // Создаем TCP-сокет
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0)
    {
        printf("Socket creation failed\n");
        return -1;
    }

    // IPv4
    server_addr.sin_family = AF_INET;

    // Указываем порт сервера
    server_addr.sin_port = htons(SERVER_PORT);

    // Преобразуем ip в бинарный вид
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Подключаемся к серверу
    if (connect(server_socket,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        printf("Connection failed\n");
        return -1;
    }

    printf("[CLIENT] Connected to server\n");

    return 0;
}

// Основной цикл работы клиента
int system_run(void)
{
    // Переменная для хранения команды сервера
    int command;

    // Пока программа работает
    while (running)
    {
        // Получаем команду от сервера     
        if (recv(server_socket,
                 &command,
                 sizeof(command),
                 0) <= 0)
        {
            printf("[CLIENT] Server disconnected\n");
            break;
        }

        // Выводим полученную команду
        printf("[CLIENT] Command = %d\n", command);

        // Обрабатываем команду
        switch (command)
        {
        case 1:
            // Запуск нагрузки
            printf("START LOAD_ потом разделить на  сpu и ram//\n");

            // Позже:
            // control_start();
            // load_cpu_start();
            // load_ram_start();

            break;

        case 2:
            // Остановка нагрузки
            printf("STOP LOAD\n");

            // Позже:
            // control_stop();

            break;
        case 3:
            // 
            printf("SEND_FILE\n");

            //????

            break;

        case 4:
            // Запрос состояния клиента
            printf("STATUS REQUEST\n");

            // Позже:
            // logger_send_status();

            break;

        case 5:
            // Завершение работы клиента
            printf("EXIT\n");

            running = 0;

            break;

        default:
            // Неизвестная команда
            printf("UNKNOWN COMMAND\n");
            break;
        }

        // Позже сюда можно добавить:
        // logger_send_log(server_socket);
    }

    return 0;
}

// завершение работы SHUTDOWN
int system_shutdown(void)
{
    // Сообщение о завершении
    printf("[CLIENT] Shutdown\n");

    // Если сокет открыт
    if (server_socket >= 0)
    {
        // Закрываем соединение
        close(server_socket);
    }

    // Позже:
    // process_shutdown();
    // logger_close();

    return 0;
}

// Точка входа программы
int main(void)
{
    // Инициализация системы
    if (system_init() != 0)
    {
        return EXIT_FAILURE;
    }

    // Подключение к серверу
    if (connect_to_server() != 0)
    {
        return EXIT_FAILURE;
    }

    // Основной цикл работы
    system_run();

    // Завершение работы
    system_shutdown();

    // Успешный выход
    return EXIT_SUCCESS;
}