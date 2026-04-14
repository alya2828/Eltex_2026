#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int isNumber(const char *str, double *value)
{
    char extra;
    if (sscanf(str, "%lf%c", value, &extra) == 1)
        return 1;
    return 0;
}

// обработка одного аргумента
void handleArg(char *arg)
{
    double val;

    if (isNumber(arg, &val)) {
        if (val == (int)val) {
            // целое число
            if (val < 0)
                printf("[PID %d] %s -> отрицательное целое, *2 = %.0f\n", getpid(), arg, val * 2);
            else
                printf("[PID %d] %s -> целое, *2 = %.0f\n", getpid(), arg, val * 2);
        } else {
            // вещественное
            if (val < 0)
                printf("[PID %d] %s -> отрицательное вещественное, *2 = %.2f\n", getpid(), arg, val * 2);
            else
                printf("[PID %d] %s -> вещественное, *2 = %.2f\n", getpid(), arg, val * 2);
        }
    } else {
        printf("[PID %d] %s -> строка\n", getpid(), arg);
    }
}

int main(int argc, char *argv[]) // сколько аргументов и сами аргументы
{
    if (argc < 2)
    {
        printf("нет аргуметов/n");
        return 0;
    }
    int mid = (argc - 1) / 2 + 1;

    pid_t pid = fork();

    if (pid == 0)
    {
        for (int i = mid; i < argc; i++)
        {
            handleArg(argv[i]);
        }
    }
    else
    {
        for (int i = 1; i < mid; i++)
        {
            handleArg(argv[i]);
        }

        wait(NULL);
    }

    return 0;
}