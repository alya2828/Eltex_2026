#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>


#define MAX_NUMS 10

union semun {
    int val;
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s file\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];

     // создаём/получаем семафор
    key_t key = ftok(filename, 1); // создали ключ 
    int semid = semget(key, 1, 0666 | IPC_CREAT);//создаст  1 дискриптор семафора семафоры это массивы значит sem[0]

    union semun arg;
    arg.val = 1;
    semctl(semid, 0, SETVAL, arg); // открыть доступ для первого семафора то есть 0-му семафору открыть = 1 место свободно
    struct sembuf lock = {0, -1, 0};//занять ресурс 
    struct sembuf unlock = {0, 1, 0};//освободить ресурс

    srand(time(NULL));

    while (1) {

        // вход в критическую секцию
        semop(semid, &lock, 1);// выполнить операцию занять ресурс

        FILE *f = fopen(filename, "a"); // а флаг добавления в конец 
        if (!f) {
            perror("fopen");
            semop(semid, &unlock, 1); //освободить ресурс если нет файла 
            continue;
        }

        //  генерируем 
        int count = rand() % MAX_NUMS + 1; //сколько чисел в строке

        for (int i = 0; i < count; i++) {
            int num = rand() % 100;
            fprintf(f, "%d ", num);
        }

        // статус 0 (не обработано)
        fprintf(f, "| 0\n");

        fclose(f);

        //  выход и
        semop(semid, &unlock, 1);//освобождаем место

        printf("Produced new line\n");

        sleep(1);
    }

    return 0;
}