#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/wait.h>

#include "shared.h"
#define SHM_NAME "/posix_shared_mem"
#define SHM_SIZE sizeof(SharedData)

SharedData *shm = NULL;
volatile sig_atomic_t running = 1;

void handle_sigint(int sig)
{
    running = 0;
    if (shm)
        shm->stop = 1;
}

void generate_numbers()
{
    shm->count = rand() % 20 + 1;

    for (int i = 0; i < shm->count; i++)
    {
        shm->numbers[i] = rand() % 100;
    }
}

void find_min_max()
{
    shm->min = shm->numbers[0];
    shm->max = shm->numbers[0];

    for (int i = 1; i < shm->count; i++)
    {
        if (shm->numbers[i] < shm->min)
            shm->min = shm->numbers[i];
        if (shm->numbers[i] > shm->max)
            shm->max = shm->numbers[i];
    }
}

int main()
{
    signal(SIGINT, handle_sigint);
    srand(time(NULL));

    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1)
    {
        perror("shm_open");
        exit(1);
    }

    //задаем размер 
    if (ftruncate(fd, SHM_SIZE) == -1)
    {
        perror("ftruncate");
        exit(1);
    }

    // отображение в память 
    shm = mmap(
        NULL,
        SHM_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0);

    if (shm == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    shm->ready = 0;
    shm->done = 0;
    shm->stop = 0;

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        exit(1);
    }

    if (pid == 0)
    {
        while (!shm->stop)
        {
            while (!shm->ready && !shm->stop)
                sleep(1);

            if (shm->stop)
                break;

            find_min_max();

            shm->done = 1;
            shm->ready = 0;
        }

        munmap(shm, SHM_SIZE);

        return 0;
    }

    int count_sets = 0;

    while (running)
    {
        generate_numbers();

        printf("Numbers: ");

        for (int i = 0; i < shm->count; i++)
            printf("%d ", shm->numbers[i]);

        printf("\n");

        shm->done = 0;
        shm->ready = 1;

        while (!shm->done && running)
            sleep(1);

        if (!running)
            break;

        printf(
            "Set %d:\nMin=%d Max=%d\n\n",
            ++count_sets,
            shm->min,
            shm->max);

        sleep(1);
    }

    shm->stop = 1;

    wait(NULL);

    printf("\nProcessed sets: %d\n", count_sets);

    munmap(shm, SHM_SIZE);

    close(fd);

    shm_unlink(SHM_NAME);

    return 0;
}
