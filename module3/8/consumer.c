#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define MAX_LINE 256

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

    char line[MAX_LINE]; // буфер для обработки строки из файла

    while (1) {
        char numbers[MAX_LINE] = ""; //строка куда копировать только числа 

        // захват
        semop(semid, &lock, 1);

        FILE *f = fopen(filename, "r+"); // флаг читатать+изменять
        if (!f) {
            perror("fopen");
            semop(semid, &unlock, 1);
            continue;
        }

        long pos; //позиция строки в файле 

        while (fgets(line, sizeof(line), f)) {

            // проверяем есть ли "| 0"
            char *status = strstr(line, "| 0");

            if (status) {
                //  сохраняем позицию строки
                pos = ftell(f) - strlen(line);// начало строки

                //  сохраняем числа (до |)
                sscanf(line, "%[^|]", numbers); // копируем все до символа

                //  возвращаемся в начало строки
                fseek(f, pos, SEEK_SET);

                //  переписываем строку с | 1
                fprintf(f, "%s| 1\n", numbers);

                break;
            }
        }

        fclose(f);

        //  освобождение
        semop(semid, &unlock, 1);

        // если ничего не нашли
        if (strlen(numbers) == 0) {
            sleep(1);
            continue;
        }

        // обработка (min/max)
        int num, min = 1000000, max = -1000000;
        char *ptr = numbers;

        while (sscanf(ptr, "%d", &num) == 1) {
            if (num < min) min = num;
            if (num > max) max = num;

            // перейти к следующему числу
            while (*ptr && *ptr != ' ') ptr++;
            while (*ptr == ' ') ptr++;
        }

        printf("Processed: %s -> min=%d max=%d\n", numbers, min, max);

        sleep(1);
    }

    return 0;
}
