#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

int main()
{
    char line[MAX_LINE];
    char *argv[MAX_ARGS];

    while (1)
    {
        printf("$ ");
        fflush(stdout); // сбросит буфер вывода сразу

        if (!fgets(line, sizeof(line), stdin))
            break;

        line[strcspn(line, "\n")] = 0;

        if (strlen(line) == 0)
            continue;

        int argc = 0;
        char *token = strtok(line, " ");
        while (token != NULL && argc < MAX_ARGS - 1)
        {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL; // последний элемент должен быть NULL для execvp
        if (strcmp(argv[0], "exit") == 0)
            break;
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork failed");
            continue;
        }

        if (pid == 0)
        {
            execvp(argv[0], argv);
            printf("Команда '%s' не найдена\n", argv[0]);
            exit(1);
        }
        else
        {

            wait(NULL);
        }
    }

    return 0;
}