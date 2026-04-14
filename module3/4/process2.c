#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 1024
#define MAX_ARGS 64
#define MAX_CMDS 16

int main()
{
    char line[MAX_LINE];

    while (1)
    {
        printf("$ ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin))
            break;

        line[strcspn(line, "\n")] = 0;

        if (strlen(line) == 0)
            continue;

        if (strcmp(line, "exit") == 0)
            break;

        // 1. делим по |
        char *commands[MAX_CMDS];
        int n = 0;

        char *cmd = strtok(line, "|");
        while (cmd != NULL && n < MAX_CMDS)
        {
            commands[n++] = cmd;
            cmd = strtok(NULL, "|");
        }

        int prev_fd = -1; // от куда читать текущей команде

        for (int i = 0; i < n; i++) // цикл по командам n = 3
        {
            int fd[2]; // создаем pipe

            if (i < n - 1) // кроме последней не передает вывод дальше 
            {
                if (pipe(fd) < 0)
                {
                    perror("pipe error");
                    exit(1);
                }
            }

            pid_t pid = fork();

            if (pid < 0)
            {
                perror("fork error");
                exit(1);
            }

            if (pid == 0) // ребенок
            {
                //  если есть вход от предыдущего pipe
                if (prev_fd != -1)
                {
                    dup2(prev_fd, STDIN_FILENO);
                    close(prev_fd);
                }

                //  если не последняя команда  вывод в pipe
                if (i < n - 1)
                {
                    dup2(fd[1], STDOUT_FILENO);
                    close(fd[0]);
                    close(fd[1]);
                }

                //  парсим аргументы команды
                char *argv[MAX_ARGS];
                int argc = 0;

                char *token = strtok(commands[i], " ");
                while (token != NULL && argc < MAX_ARGS - 1)
                {
                    argv[argc++] = token;
                    token = strtok(NULL, " ");
                }
                argv[argc] = NULL;

                //  ОБРАБОТКА < и >
                for (int j = 0; argv[j] != NULL; j++)
                {
                    // <
                    if (strcmp(argv[j], "<") == 0)
                    {
                        int fd_in = open(argv[j + 1], O_RDONLY);
                        if (fd_in < 0)
                        {
                            perror("open input");
                            exit(1);
                        }

                        dup2(fd_in, STDIN_FILENO);
                        close(fd_in);

                        argv[j] = NULL;
                        break;
                    }

                    // >
                    if (strcmp(argv[j], ">") == 0)
                    {
                        int fd_out = open(argv[j + 1],
                                          O_WRONLY | O_CREAT | O_TRUNC, 0666);
                        if (fd_out < 0)
                        {
                            perror("open output");
                            exit(1);
                        }

                        dup2(fd_out, STDOUT_FILENO);
                        close(fd_out);

                        argv[j] = NULL;
                        break;
                    }
                }

                execvp(argv[0], argv);
                perror("exec failed");
                exit(1);
            }
            else //  родитель
            {
                if (prev_fd != -1)
                    close(prev_fd);

                if (i < n - 1)
                {
                    close(fd[1]);
                    prev_fd = fd[0]; // сохранить "предыдущий pipe"
                }
            }
        }

        //  ждём все процессы
        for (int i = 0; i < n; i++)
            wait(NULL);
    }

    return 0;
}
