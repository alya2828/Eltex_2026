#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <sys/wait.h>
#include "shared.h"

#define SHM_SIZE sizeof(SharedData)

int shmid;
SharedData *shm;
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

    key_t key = ftok("shared.h", 23);

    shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1)
    {
        perror("shmget");
        exit(1);
    }

    shm = (SharedData *)shmat(shmid, NULL, 0);
    if (shm == (void *)-1)
    {
        perror("shmat");
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
                usleep(1000);

            if (shm->stop)
                break;

            find_min_max();

            shm->done = 1;
            shm->ready = 0;
        }

        shmdt(shm);
        exit(0);
    }

    int count_sets = 0;

    while (running)
    {

        generate_numbers();
        printf("Numbers: ");
        for (int i = 0; i < shm->count; i++)
        {
            printf("%d ", shm->numbers[i]);
        }
        printf("\n");

        shm->ready = 1;

        shm->ready = 1;
        shm->done = 0;

        while (!shm->done && running)
            usleep(1000);

        if (!running)
            break;

        printf("Set %d:\n", ++count_sets);
        printf("Min = %d, Max = %d\n\n", shm->min, shm->max);

        sleep(1);
    }

    shm->stop = 1;

    wait(NULL);

    printf("\nProcessed sets: %d\n", count_sets);

    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}