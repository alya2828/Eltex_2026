#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>
#include <signal.h>

#define MAX_LINE 256
#define MAX_NUMS 10

volatile sig_atomic_t running = 1;

void handle_sigint(int sig)
{
    running = 0;
}
void process_line(char *line)
{
    int num, min, max;
    char *ptr = line;

    if (sscanf(ptr, "%d", &num) != 1)
        return;

    min = max = num;

    while (sscanf(ptr, "%d", &num) == 1)
    {
        if (num < min) min = num;
        if (num > max) max = num;

        while (*ptr && *ptr != ' ') ptr++;
        while (*ptr == ' ') ptr++;
    }

    printf("Processed: %s -> min=%d max=%d\n", line, min, max);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s file\n", argv[0]);
        return 1;
    }

    srand(time(NULL));

    char *filename = argv[1];
    sem_t *sem = sem_open("/mysem", O_CREAT, 0666, 1);

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        exit(1);
    }

    if (pid > 0)
    {
        signal(SIGINT, handle_sigint);
        while (running)
        {
            sem_wait(sem);//захватываем ресурс

            FILE *f = fopen(filename, "a");

            int count = rand() % MAX_NUMS + 1;
            for (int i = 0; i < count; i++)
            {
                fprintf(f, "%d ", rand() % 100);
            }

            fprintf(f, "| 0\n");
            fclose(f);

            printf("Produced line\n");

            sem_post(sem);// освобождаем ресурс

            sleep(1);
        }

        wait(NULL);
        sem_close(sem);//закрываем сем в текущем процессе но не убираем из системы
        sem_unlink("/mysem");//удалить сем из системы
        printf("Parent finished\n");
    }
    else
    {
        signal(SIGINT, handle_sigint);
        while (running)
        {
            sem_wait(sem);

            FILE *f = fopen(filename, "r+");

            char line[MAX_LINE];
            long pos;
            char numbers[MAX_LINE];

            int found = 0;

            while (fgets(line, sizeof(line), f))
            {
                char *status = strstr(line, "| 0");

                if (status)
                {
                    found = 1;

                    pos = ftell(f) - strlen(line);

                    sscanf(line, "%[^|]", numbers);

                    fseek(f, pos, SEEK_SET);
                    fprintf(f, "%s| 1\n", numbers);

                    break;
                }
            }

            fclose(f);
            sem_post(sem);

            if (found)
            {
                process_line(numbers);
            }
            else
            {
                sleep(1);
            }
        }
    }

    return 0;
}